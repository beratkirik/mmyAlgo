#include "simplog.h"
extern "C" {
#include "fix_common.h"
#include "libtrading/compat.h"
#include "libtrading/array.h"
#include "libtrading/time.h"
#include "libtrading/die.h"
}
#include <netinet/tcp.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <cinttypes>
#include <libgen.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <cfloat>
#include <netdb.h>
#include <cstdio>
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <mysql/mysql.h>
extern "C" {
#include "fix_client.h"
#include "test.h"
}

#ifdef __cplusplus
extern "C" {
#endif

static const char *program;
static sig_atomic_t stop;

static struct fix_client_function fix_client_functions[] = {
        [FIX_CLIENT_SCRIPT] = {
                .fix_session_initiate	= fix_client_script,
                .mode			= FIX_CLIENT_SCRIPT,
        },
        [FIX_CLIENT_SESSION] = {
                .fix_session_initiate	= fix_client_session,
                .mode			= FIX_CLIENT_SESSION,
        },
        [FIX_CLIENT_ORDER] = {
                .fix_session_initiate	= fix_client_order,
                .mode			= FIX_CLIENT_ORDER,
        },
};

static void signal_handler(int signum)
{
    if (signum == SIGINT)
        stop = 1;
}

static int fix_logout_send(struct fix_session *session, const char *text)
{
    struct fix_field fields[] = {
            FIX_STRING_FIELD(Text, text),
    };
    long nr_fields = ARRAY_SIZE(fields);
    struct fix_message logout_msg{};

    if (!text)
        nr_fields--;
//  struct yapısı değiştirildi !!
    struct fix_message fm{};
    fm.type		    = FIX_MSG_TYPE_LOGOUT;
    fm.nr_fields	= nr_fields;
    fm.fields		= fields;
    logout_msg	= fm;
    //logout_msg	= (struct fix_message) {
    //        .type		= FIX_MSG_TYPE_LOGOUT,
    //        .nr_fields	= static_cast<unsigned long>(nr_fields),
    //        .fields		= fields,
    //};

    return fix_session_send(session, &logout_msg, 0);
}

static int fix_client_logout(struct fix_session *session, const char *text, bool grace)
{
    int ret;

    if (grace)
        ret = fix_session_logout(session, text);
    else
        ret = fix_logout_send(session, text);

    if (ret)
        fprintf(stderr, "Client Logout FAILED\n");
    else
        fprintf(stdout, "Client Logout OK\n");

    return ret;
}

static int fix_client_script(struct fix_session_cfg *cfg, struct fix_client_arg *arg)
{
    struct fcontainer *s_container = nullptr;
    struct fcontainer *c_container = nullptr;
    struct fix_session *session = nullptr;
    struct felem *expected_elem;
    struct felem *tosend_elem;
    struct fix_message *msg;
    FILE *stream = nullptr;
    int ret = -1;

    if (!arg->script) {
        fprintf(stderr, "No script is specified\n");
        goto exit;
    }

    stream = fopen(arg->script, "r");
    if (!stream) {
        fprintf(stderr, "Opening %s failed: %s\n",
                arg->script, strerror(errno));
        goto exit;
    }

    session = fix_session_new(cfg);
    if (!session) {
        fprintf(stderr, "FIX session cannot be created\n");
        goto exit;
    }

    s_container = fcontainer_new();
    if (!s_container) {
        fprintf(stderr, "Cannot allocate container\n");
        goto exit;
    }

    c_container = fcontainer_new();
    if (!c_container) {
        fprintf(stderr, "Cannot allocate container\n");
        goto exit;
    }

    if (script_read(stream, s_container, c_container)) {
        fprintf(stderr, "Invalid script: %s\n", arg->script);
        goto exit;
    }

    ret = fix_session_logon(session);
    if (ret) {
        fprintf(stderr, "Client Logon FAILED\n");
        goto exit;
    }

    fprintf(stdout, "Client Logon OK\n");

    expected_elem = cur_elem(s_container);
    tosend_elem = cur_elem(c_container);

    while (tosend_elem) {
        if (tosend_elem->msg.msg_seq_num)
            fix_session_send(session, &tosend_elem->msg, FIX_SEND_FLAG_PRESERVE_MSG_NUM);
        else
            fix_session_send(session, &tosend_elem->msg, 0);

        if (!expected_elem)
            goto next;

        retry:
        if (fix_session_recv(session, &msg, FIX_RECV_FLAG_MSG_DONTWAIT) <= 0)
            goto retry;

        fprintf(stdout, "< ");
        fprintmsg_iov(stdout, msg);

        if (fmsgcmp(&expected_elem->msg, msg)) {
            fprintf(stderr, "Client: messages differ\n");
            fprintmsg(stderr, &expected_elem->msg);
            fprintmsg(stderr, msg);
            break;
        }

        if (fix_message_type_is(msg, FIX_MSG_TYPE_LOGOUT)) {
            ret = fix_client_logout(session, nullptr, false);
            goto exit;
        }

        next:
        expected_elem = next_elem(s_container);
        tosend_elem = next_elem(c_container);
    }

    ret = tosend_elem ? -1 : 0;
    if (ret)
        goto exit;

    ret = fix_client_logout(session, nullptr, true);

    exit:
    fcontainer_free(c_container);
    fcontainer_free(s_container);
    fix_session_free(session);

    if (stream)
        fclose(stream);

    return ret;
}

static int fix_client_session(struct fix_session_cfg *cfg, struct fix_client_arg *arg)
{
    struct fix_session *session = nullptr;
    struct timespec cur{}, prev{};
    struct fix_message *msg;
    int ret = -1;
    int diff;

    if (signal(SIGINT, signal_handler) == SIG_ERR) {
        fprintf(stderr, "Unable to register a signal handler\n");
        goto exit;
    }

    session	= fix_session_new(cfg);
    if (!session) {
        fprintf(stderr, "FIX session cannot be created\n");
        goto exit;
    }

    ret = fix_session_logon(session);
    if (ret) {
        fprintf(stderr, "Client Logon FAILED\n");
        goto exit;
    }

    fprintf(stdout, "Client Logon OK\n");

    clock_gettime(CLOCK_MONOTONIC, &prev);

    while (!stop && session->active) {
        clock_gettime(CLOCK_MONOTONIC, &cur);
        diff = cur.tv_sec - prev.tv_sec;

        if (diff > 0.1 * session->heartbtint) {
            prev = cur;

            if (!fix_session_keepalive(session, &cur)) {
                stop = 1;
                break;
            }
        }

        if (fix_session_time_update(session)) {
            stop = 1;
            break;
        }

        if (fix_session_recv(session, &msg, FIX_RECV_FLAG_MSG_DONTWAIT) <= 0) {
            fprintmsg(stdout, msg);

            if (fix_session_admin(session, msg))
                continue;

            switch (msg->type) {
                case FIX_MSG_TYPE_LOGOUT:
                    stop = 1;
                    break;
                default:
                    stop = 1;
                    break;
            }
        }
    }

    if (session->active) {
        ret = fix_session_logout(session, nullptr);
        if (ret) {
            fprintf(stderr, "Client Logout FAILED\n");
            goto exit;
        }
    }

    fprintf(stdout, "Client Logout OK\n");

    exit:
    fix_session_free(session);

    return ret;
}
int coid = 5000;
static unsigned long fix_new_order_single_fields(struct fix_session *session, struct fix_field *fields)
{
    unsigned long nr = 0;

    fields[nr++] = FIX_STRING_FIELD(TransactTime, session->str_now);
    fields[nr++] = FIX_INT_FIELD(ClOrdID, coid++);
    fields[nr++] = FIX_STRING_FIELD(Symbol, "AKBNK");
    fields[nr++] = FIX_FLOAT_FIELD(OrderQty, 500);
    fields[nr++] = FIX_STRING_FIELD(OrdType, "2");
    fields[nr++] = FIX_STRING_FIELD(Side, "1");
    fields[nr++] = FIX_FLOAT_FIELD(Price, 6);

    return nr;
}

static int fix_client_order(struct fix_session_cfg *cfg, struct fix_client_arg *arg)
{
    double min_usec, avg_usec, max_usec, total_usec;
    struct fix_session *session = nullptr;
    struct fix_field *fields = nullptr;
    struct fix_message *msg;
    FILE *file = nullptr;
    unsigned long nr;
    int ret = -1;
    int orders;
    int i;

    // mysql connection
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;

    char const *server = "localhost";
    char const *user = "root";
    char const *password = "berat";
    char const *database = "myAlgo";

    conn = mysql_init(nullptr);
    /* Connect to database */
    if (!mysql_real_connect(conn, server,
                            user, password, database, 0, nullptr, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    char query[500] = {0};
    // mysql connection

    if (!arg)
        goto exit;

    orders = arg->orders;

    if (arg->output) {
        file = fopen(arg->output, "w");

        if (!file) {
            fprintf(stderr, "Cannot open a file %s\n", arg->output);
            goto exit;
        }
    }

    session	= fix_session_new(cfg);
    fprintf(stderr,"Session is taken.\n");
    if (!session) {
        fprintf(stderr, "FIX session cannot be created\n");
        goto exit;
    }
    fprintf(stderr,"Session is valid\n");
    ret = fix_session_logon(session);
    if (ret) {
        fprintf(stderr, "Client Logon FAILED\n");
        goto exit;
    }
    fprintf(stderr,"Session is valid\n");
    fprintf(stdout, "Client Logon OK\n");

    ret = -1;

    fields = static_cast<fix_field *>(calloc(FIX_MAX_FIELD_NUMBER, sizeof(struct fix_field)));
    if (!fields) {
        fprintf(stderr, "Cannot allocate memory\n");
        goto exit;
    }
//    fprintf(stderr, "System can allocate memory\n");


    min_usec	= DBL_MAX;
    max_usec	= 0;
    total_usec	= 0;

    for (i = 0; i < arg->warmup_orders; i++) {
        fix_session_new_order_single(session, fields, nr);

        retry_warmup:
        if (fix_session_recv(session, &msg, MSG_DONTWAIT) <= 0)
            goto retry_warmup;

        if (!fix_message_type_is(msg, FIX_MSG_TYPE_EXECUTION_REPORT))
            goto retry_warmup;
    }

    for (i = 0; i < orders; i++) {
        struct timespec before{}, after{};
        uint64_t elapsed_usec;
        clock_gettime(CLOCK_MONOTONIC, &before);
        nr = fix_new_order_single_fields(session, fields);

        fix_session_new_order_single(session, fields, nr);

        //mysql insert başlangıç
        snprintf(query,500,"INSERT INTO myFixLogs(clientOrderID, orderStatus, stockName, orderQty , orderPrice) VALUES ('%ld','1','%s' , '%f' , '%f')", fields[1].int_value, fields[2].string_value, fields[3].float_value, fields[6].float_value);
        if (mysql_query(conn, query)) {
            fprintf(stderr, "%s\n", mysql_error(conn));
            }
        // mysql insert bitiş
        // client order id arttirmak icin eklenmistir
        //coid++;
        // client order id ekleme işlemi biter.

        retry:
        if (fix_session_recv(session, &msg, FIX_RECV_FLAG_MSG_DONTWAIT) <= 0)
            goto retry;

        if (!fix_message_type_is(msg, FIX_MSG_TYPE_EXECUTION_REPORT))
            goto retry;

        // test
        fprintmsg_iov(stdout, msg);
        // test biter

        clock_gettime(CLOCK_MONOTONIC, &after);

        elapsed_usec = timespec_delta(&before, &after) / 1000;

        total_usec += elapsed_usec;

        min_usec = fmin(min_usec, elapsed_usec);
        max_usec = fmax(max_usec, elapsed_usec);

        if (file)
            fprintf(file, "%" PRIu64 "\n", elapsed_usec);
    }

    //mysql kapat
    res = mysql_use_result(conn);
    mysql_free_result(res);
    mysql_close(conn);
    // mysql kapat

    avg_usec = total_usec / orders;

    fprintf(stdout, "Messages sent: %d\n", orders);
    fprintf(stdout, "Round-trip time: min/avg/max = %.1lf/%.1lf/%.1lf μs\n", min_usec, avg_usec, max_usec);


    if (session->active) {
        ret = fix_session_logout(session, nullptr);
        if (ret) {
            fprintf(stderr, "Client Logout FAILED\n");
            goto exit;
        }
    }

    fprintf(stdout, "Client Logout OK\n");

    exit:
    fix_session_free(session);
    free(fields);

    if (file)
        fclose(file);

    return ret;
}

static void usage()
{
    printf("\n usage: %s [-m mode] [-d dialect] [-f filename] [-n orders] [-s sender-comp-id] [-t target-comp-id] [-r password] [-w warmup orders] -h hostname -p port\n\n", program);

    exit(EXIT_FAILURE);
}

static int socket_setopt(int sockfd, int level, int optname, int optval)
{
    return setsockopt(sockfd, level, optname, (void *) &optval, sizeof(optval));
}

static enum fix_client_mode strclientmode(const char *mode)
{
    int m;

    if (!strcmp(mode, "session"))
        return FIX_CLIENT_SESSION;
    else if (!strcmp(mode, "script"))
        return FIX_CLIENT_SCRIPT;
    else if (!strcmp(mode, "order"))
        return FIX_CLIENT_ORDER;

    int ival = 0;
    sscanf(mode, "%d", &ival);
    m = static_cast<fix_client_mode>(ival);
    if (m != 1)
        return FIX_CLIENT_SCRIPT;

    switch (m) {
        case FIX_CLIENT_SESSION:
        case FIX_CLIENT_SCRIPT:
        case FIX_CLIENT_ORDER:
            return static_cast<fix_client_mode>(m);
        default:
            break;
    }

    return FIX_CLIENT_SCRIPT;
}

static enum fix_version strversion(const char *dialect)
{
    if (!strcmp(dialect, "fix42"))
        return FIX_4_2;
    else if (!strcmp(dialect, "fix43"))
        return FIX_4_3;
    else if (!strcmp(dialect, "fix44"))
        return FIX_4_4;

    return FIX_4_4;
}

int main(int argc, char *argv[])
{
    simplog.setLogFile("/home/berat/CLionProjects/myAlgo/log.txt");
    enum fix_client_mode mode = FIX_CLIENT_SCRIPT;
    enum fix_version version = FIX_4_4;
    const char *target_comp_id = nullptr;
    const char *sender_comp_id = nullptr;
    struct fix_client_arg arg = {nullptr};
    const char *password = nullptr;
    struct fix_session_cfg cfg{};
    const char *host = nullptr;
    struct sockaddr_in sa{};
    int saved_errno = 0;
    struct hostent *he;
    int port = 0;
    int ret = 0;
    char **ap;
    int opt;



    while ((opt = getopt(argc, argv, "f:h:p:d:s:t:m:n:o:r:w:")) != -1) {
        switch (opt) {
            case 'd':
                version = strversion(optarg);
                break;
            case 'n':
                arg.orders = atoi(optarg);
                break;
            case 's':
                sender_comp_id = optarg;
                break;
            case 't':
                target_comp_id = optarg;
                break;
            case 'r':
                password = optarg;
                break;
            case 'm':
                mode = strclientmode(optarg);
                break;
            case 'p':
                port = atoi(optarg);
                break;
            case 'f':
                arg.script = optarg;
                break;
            case 'o':
                arg.output = optarg;
                break;
            case 'h':
                host = optarg;
                break;
            case 'w':
                arg.warmup_orders = atoi(optarg);
                break;
            default: /* '?' */
                usage();
        }
    }


    if (!port || !host)
        usage();

    fix_session_cfg_init(&cfg);

    cfg.dialect	= &fix_dialects[version];

    if (!password) {
        memset(cfg.password, 0, ARRAY_SIZE(cfg.password));
    } else {
        strncpy(cfg.password, password, ARRAY_SIZE(cfg.password));
    }

    if (!sender_comp_id) {
        strncpy(cfg.sender_comp_id, "BUYSIDE", ARRAY_SIZE(cfg.sender_comp_id));
    } else {
        strncpy(cfg.sender_comp_id, sender_comp_id, ARRAY_SIZE(cfg.sender_comp_id));
    }

    if (!target_comp_id) {
        strncpy(cfg.target_comp_id, "SELLSIDE", ARRAY_SIZE(cfg.target_comp_id));
    } else {
        strncpy(cfg.target_comp_id, target_comp_id, ARRAY_SIZE(cfg.target_comp_id));
    }

    he = gethostbyname(host);
    if (!he)
        error("Unable to look up %s (%s)", host, hstrerror(h_errno));

    for (ap = he->h_addr_list; *ap; ap++) {
        cfg.sockfd = socket(he->h_addrtype, SOCK_STREAM, IPPROTO_TCP);
        if (cfg.sockfd < 0) {
            saved_errno = errno;
            continue;
        }

        sa = (struct sockaddr_in) {
                .sin_family		= static_cast<sa_family_t>(he->h_addrtype),
                .sin_port		= htons(port),
        };
        memcpy(&sa.sin_addr, *ap, he->h_length);

        if (connect(cfg.sockfd, (const struct sockaddr *)&sa, sizeof(struct sockaddr_in)) < 0) {
            saved_errno = errno;
            close(cfg.sockfd);
            cfg.sockfd = -1;
            continue;
        }
        break;
    }

    if (cfg.sockfd < 0)
        error("Unable to connect to a socket (%s)", strerror(saved_errno));

    if (socket_setopt(cfg.sockfd, IPPROTO_TCP, TCP_NODELAY, 1) < 0)
        die("cannot set socket option TCP_NODELAY");

    cfg.heartbtint = 15;

    switch (mode) {
        case FIX_CLIENT_SCRIPT:
            ret = fix_client_functions[mode].fix_session_initiate(&cfg, &arg);
            break;
        case FIX_CLIENT_ORDER:
            ret = fix_client_functions[mode].fix_session_initiate(&cfg, &arg);
            break;
        case FIX_CLIENT_SESSION:
            ret = fix_client_functions[mode].fix_session_initiate(&cfg, nullptr);
            break;
        default:
            error("Invalid mode");
    }

    shutdown(cfg.sockfd, SHUT_RDWR);

    if (close(cfg.sockfd) < 0)
        die("close");

    return ret;
}

#ifdef __cplusplus
}
#endif