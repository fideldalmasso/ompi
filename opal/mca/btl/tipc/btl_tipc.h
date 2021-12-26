/* -*- Mode: C; c-basic-offset:4 ; indent-tabs-mode:nil -*- */
/*
 * Copyright (c) 2004-2007 The Trustees of Indiana University and Indiana
 *                         University Research and Technology
 *                         Corporation.  All rights reserved.
 * Copyright (c) 2004-2016 The University of Tennessee and The University
 *                         of Tennessee Research Foundation.  All rights
 *                         reserved.
 * Copyright (c) 2004-2005 High Performance Computing Center Stuttgart,
 *                         University of Stuttgart.  All rights reserved.
 * Copyright (c) 2004-2005 The Regents of the University of California.
 *                         All rights reserved.
 * Copyright (c) 2010-2011 Cisco Systems, Inc.  All rights reserved.
 * Copyright (c) 2014-2016 Research Organization for Information Science
 *                         and Technology (RIST). All rights reserved.
 * Copyright (c) 2014-2015 Los Alamos National Security, LLC. All rights
 *                         reserved.
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates.  All Rights
 *                         reserved.
 * $COPYRIGHT$
 *
 * Additional copyrights may follow
 *
 * $HEADER$
 */
/**
 * @file
 */
#define FIDEL 0
#define SERVICE_TYPE  8888
#define SERVICE_INSTANCE  17

#ifndef MCA_BTL_TIPC_H
#define MCA_BTL_TIPC_H

#include "opal_config.h"
#ifdef HAVE_SYS_TYPES_H
#    include <sys/types.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#    include <sys/socket.h>
#endif
#ifdef HAVE_NETINET_IN_H
#    include <netinet/in.h>
#endif
#ifdef HAVE_UNISTD_H
#    include <unistd.h>
#endif

/* Open MPI includes */
#include "opal/class/opal_free_list.h"
#include "opal/class/opal_hash_table.h"
#include "opal/mca/btl/base/base.h"
#include "opal/mca/btl/btl.h"
#include "opal/mca/mpool/mpool.h"
#include "opal/util/event.h"
#include "opal/util/fd.h"

#define MCA_BTL_TIPC_STATISTICS 0
BEGIN_C_DECLS

extern opal_event_base_t *mca_btl_tipc_event_base;

#define MCA_BTL_TIPC_COMPLETE_FRAG_SEND(frag)                                            \
    do {                                                                                \
        int btl_ownership = (frag->base.des_flags & MCA_BTL_DES_FLAGS_BTL_OWNERSHIP);   \
        if (frag->base.des_flags & MCA_BTL_DES_SEND_ALWAYS_CALLBACK) {                  \
            frag->base.des_cbfunc(&frag->endpoint->endpoint_btl->super, frag->endpoint, \
                                  &frag->base, frag->rc);                               \
        }                                                                               \
        if (btl_ownership) {                                                            \
            MCA_BTL_TIPC_FRAG_RETURN(frag);                                              \
        }                                                                               \
    } while (0)

extern opal_list_t mca_btl_tipc_ready_frag_pending_queue;
extern opal_mutex_t mca_btl_tipc_ready_frag_mutex;
extern int mca_btl_tipc_pipe_to_progress[2];
extern int mca_btl_tipc_progress_thread_trigger;

#define MCA_BTL_TIPC_CRITICAL_SECTION_ENTER(name) opal_mutex_atomic_lock((name))
#define MCA_BTL_TIPC_CRITICAL_SECTION_LEAVE(name) opal_mutex_atomic_unlock((name))

#define MCA_BTL_TIPC_ACTIVATE_EVENT(event, value)                                          \
    do {                                                                                  \
        if (0 < mca_btl_tipc_progress_thread_trigger) {                                    \
            opal_event_t *_event = (opal_event_t *) (event);                              \
            (void) opal_fd_write(mca_btl_tipc_pipe_to_progress[1], sizeof(opal_event_t *), \
                                 &_event);                                                \
        } else {                                                                          \
            opal_event_add(event, (value));                                               \
        }                                                                                 \
    } while (0)

/**
 * TIPC BTL component.
 */

struct mca_btl_tipc_component_t {
    mca_btl_base_component_3_0_0_t super; /**< base BTL component */
    uint32_t tipc_addr_count;              /**< total number of addresses */
    uint32_t tipc_num_btls;      /**< number of interfaces available to the TIPC component */
    unsigned int tipc_num_links; /**< number of logical links per physical device */
    struct mca_btl_tipc_module_t **tipc_btls; /**< array of available BTL modules */
    opal_list_t local_ifs;                  /**< opal list of local opal_if_t interfaces */
    int tipc_free_list_num;                  /**< initial size of free lists */
    int tipc_free_list_max;                  /**< maximum size of free lists */
    int tipc_free_list_inc;       /**< number of elements to alloc when growing free lists */
    int tipc_endpoint_cache;      /**< amount of cache on each endpoint */
    opal_proc_table_t tipc_procs; /**< hash table of tipc proc structures */
    opal_mutex_t tipc_lock;       /**< lock for accessing module state */
    opal_list_t tipc_events;

    opal_event_t tipc_recv_event;    /**< recv event for IPv4 listen socket */
    int tipc_listen_sd;              /**< IPv4 listen socket for incoming connection requests */
    unsigned short tipc_listen_port; /**< IPv4 listen port */
    int tipc_port_min;               /**< IPv4 minimum port */
    int tipc_port_range;             /**< IPv4 port range */
// #if OPAL_ENABLE_IPV6
//     opal_event_t tipc6_recv_event;    /**< recv event for IPv6 listen socket */
//     int tipc6_listen_sd;              /**< IPv6 listen socket for incoming connection requests */
//     unsigned short tipc6_listen_port; /**< IPv6 listen port */
//     int tipc6_port_min;               /**< IPv4 minimum port */
//     int tipc6_port_range;             /**< IPv4 port range */
// #endif
    /* Port range restriction */

    char *tipc_if_include;   /**< comma seperated list of interface to include */
    char *tipc_if_exclude;   /**< comma seperated list of interface to exclude */
    int tipc_sndbuf;         /**< socket sndbuf size */
    int tipc_rcvbuf;         /**< socket rcvbuf size */
    int tipc_disable_family; /**< disabled AF_family */

    /* free list of fragment descriptors */
    opal_free_list_t tipc_frag_eager;
    opal_free_list_t tipc_frag_max;
    opal_free_list_t tipc_frag_user;

    int tipc_enable_progress_thread; /** Support for tipc progress thread flag */

    opal_event_t tipc_recv_thread_async_event;
    opal_mutex_t tipc_frag_eager_mutex;
    opal_mutex_t tipc_frag_max_mutex;
    opal_mutex_t tipc_frag_user_mutex;
    /* Do we want to use TIPC_NODELAY? */
    int tipc_not_use_nodelay;

    /* do we want to warn on all excluded interfaces
     * that are not found?
     */
    bool report_all_unfound_interfaces;
};
typedef struct mca_btl_tipc_component_t mca_btl_tipc_component_t;

OPAL_MODULE_DECLSPEC extern mca_btl_tipc_component_t mca_btl_tipc_component;

/**
 * BTL Module Interface
 */
struct mca_btl_tipc_module_t {
    mca_btl_base_module_t super;        /**< base BTL interface */
    uint32_t btl_index;                 /**< Local BTL module index, used for vertex
                                             data and used as a hash key when
                                             solving module matching problem */
    uint16_t tipc_ifkindex;              /** <BTL kernel interface index */
    struct sockaddr_storage tipc_ifaddr; /**< First address
                                           discovered for this
                                           interface, bound as
                                           sending address for this
                                           BTL */
    uint32_t tipc_ifmask;                /**< BTL interface netmask */

    opal_mutex_t tipc_endpoints_mutex;
    opal_list_t tipc_endpoints;

    mca_btl_base_module_error_cb_fn_t tipc_error_cb; /**< Upper layer error callback */
#if MCA_BTL_TIPC_STATISTICS
    size_t tipc_bytes_sent;
    size_t tipc_bytes_recv;
    size_t tipc_send_handler;
#endif
};
typedef struct mca_btl_tipc_module_t mca_btl_tipc_module_t;
extern mca_btl_tipc_module_t mca_btl_tipc_module;

#define CLOSE_THE_SOCKET(socket)                                                                   \
    {                                                                                              \
        OPAL_OUTPUT_VERBOSE((20, opal_btl_base_framework.framework_output, "CLOSE FD %d at %s:%d", \
                             socket, __FILE__, __LINE__));                                         \
        (void) shutdown(socket, SHUT_RDWR);                                                        \
        (void) close(socket);                                                                      \
    }

/**
 * TIPC component initialization.
 *
 * @param num_btl_modules (OUT)           Number of BTLs returned in BTL array.
 * @param allow_multi_user_threads (OUT)  Flag indicating wether BTL supports user threads (TRUE)
 * @param have_hidden_threads (OUT)       Flag indicating wether BTL uses threads (TRUE)
 */
extern mca_btl_base_module_t **mca_btl_tipc_component_init(int *num_btl_modules,
                                                          bool allow_multi_user_threads,
                                                          bool have_hidden_threads);

/**
 * Cleanup any resources held by the BTL.
 *
 * @param btl  BTL instance.
 * @return     OPAL_SUCCESS or error status on failure.
 */

extern int mca_btl_tipc_finalize(struct mca_btl_base_module_t *btl);

/**
 * PML->BTL notification of change in the process list.
 *
 * @param btl (IN)
 * @param nprocs (IN)     Number of processes
 * @param procs (IN)      Set of processes
 * @param peers (OUT)     Set of (optional) peer addressing info.
 * @param peers (IN/OUT)  Set of processes that are reachable via this BTL.
 * @return     OPAL_SUCCESS or error status on failure.
 *
 */

extern int mca_btl_tipc_add_procs(struct mca_btl_base_module_t *btl, size_t nprocs,
                                 struct opal_proc_t **procs, struct mca_btl_base_endpoint_t **peers,
                                 opal_bitmap_t *reachable);

/**
 * PML->BTL notification of change in the process list.
 *
 * @param btl (IN)     BTL instance
 * @param nproc (IN)   Number of processes.
 * @param procs (IN)   Set of processes.
 * @param peers (IN)   Set of peer data structures.
 * @return             Status indicating if cleanup was successful
 *
 */

extern int mca_btl_tipc_del_procs(struct mca_btl_base_module_t *btl, size_t nprocs,
                                 struct opal_proc_t **procs,
                                 struct mca_btl_base_endpoint_t **peers);

/**
 * Initiate an asynchronous send.
 *
 * @param btl (IN)         BTL module
 * @param endpoint (IN)    BTL addressing information
 * @param descriptor (IN)  Description of the data to be transfered
 * @param tag (IN)         The tag value used to notify the peer.
 */

extern int mca_btl_tipc_send(struct mca_btl_base_module_t *btl,
                            struct mca_btl_base_endpoint_t *btl_peer,
                            struct mca_btl_base_descriptor_t *descriptor, mca_btl_base_tag_t tag);

/**
 * Initiate an asynchronous put.
 */

int mca_btl_tipc_put(mca_btl_base_module_t *btl, struct mca_btl_base_endpoint_t *endpoint,
                    void *local_address, uint64_t remote_address,
                    mca_btl_base_registration_handle_t *local_handle,
                    mca_btl_base_registration_handle_t *remote_handle, size_t size, int flags,
                    int order, mca_btl_base_rdma_completion_fn_t cbfunc, void *cbcontext,
                    void *cbdata);

/**
 * Initiate an asynchronous get.
 */

int mca_btl_tipc_get(mca_btl_base_module_t *btl, struct mca_btl_base_endpoint_t *endpoint,
                    void *local_address, uint64_t remote_address,
                    mca_btl_base_registration_handle_t *local_handle,
                    mca_btl_base_registration_handle_t *remote_handle, size_t size, int flags,
                    int order, mca_btl_base_rdma_completion_fn_t cbfunc, void *cbcontext,
                    void *cbdata);

/**
 * Allocate a descriptor with a segment of the requested size.
 * Note that the BTL layer may choose to return a smaller size
 * if it cannot support the request.
 *
 * @param btl (IN)      BTL module
 * @param size (IN)     Request segment size.
 */

extern mca_btl_base_descriptor_t *mca_btl_tipc_alloc(struct mca_btl_base_module_t *btl,
                                                    struct mca_btl_base_endpoint_t *endpoint,
                                                    uint8_t order, size_t size, uint32_t flags);

/**
 * Return a segment allocated by this BTL.
 *
 * @param btl (IN)      BTL module
 * @param descriptor (IN)  Allocated descriptor.
 */

extern int mca_btl_tipc_free(struct mca_btl_base_module_t *btl, mca_btl_base_descriptor_t *des);

/**
 * Prepare a descriptor for send/rdma using the supplied
 * convertor. If the convertor references data that is contigous,
 * the descriptor may simply point to the user buffer. Otherwise,
 * this routine is responsible for allocating buffer space and
 * packing if required.
 *
 * @param btl (IN)          BTL module
 * @param endpoint (IN)     BTL peer addressing
 * @param convertor (IN)    Data type convertor
 * @param reserve (IN)      Additional bytes requested by upper layer to precede user data
 * @param size (IN/OUT)     Number of bytes to prepare (IN), number of bytes actually prepared (OUT)
 */

mca_btl_base_descriptor_t *mca_btl_tipc_prepare_src(struct mca_btl_base_module_t *btl,
                                                   struct mca_btl_base_endpoint_t *peer,
                                                   struct opal_convertor_t *convertor,
                                                   uint8_t order, size_t reserve, size_t *size,
                                                   uint32_t flags);

extern void mca_btl_tipc_dump(struct mca_btl_base_module_t *btl,
                             struct mca_btl_base_endpoint_t *endpoint, int verbose);

/*
 * A blocking send on a non-blocking socket. Used to send the small
 * amount of connection information that identifies the endpoints
 * endpoint.
 */
int mca_btl_tipc_send_blocking(int sd, const void *data, size_t size);

/*
 * A blocking recv for both blocking and non-blocking socket.
 * Used to receive the small amount of connection information
 * that identifies the endpoints
 *
 * when the socket is blocking (the caller introduces timeout)
 * which happens during initial handshake otherwise socket is
 * non-blocking most of the time.
 */
int mca_btl_tipc_recv_blocking(int sd, void *data, size_t size);

END_C_DECLS
#endif
