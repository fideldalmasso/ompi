#
# Copyright (c) 2004-2005 The Trustees of Indiana University and Indiana
#                         University Research and Technology
#                         Corporation.  All rights reserved.
# Copyright (c) 2004-2020 The University of Tennessee and The University
#                         of Tennessee Research Foundation.  All rights
#                         reserved.
# Copyright (c) 2004-2009 High Performance Computing Center Stuttgart,
#                         University of Stuttgart.  All rights reserved.
# Copyright (c) 2004-2005 The Regents of the University of California.
#                         All rights reserved.
# Copyright (c) 2009-2014 Cisco Systems, Inc.  All rights reserved.
# Copyright (c) 2013      NVIDIA Corporation.  All rights reserved.
# Copyright (c) 2017      IBM Corporation.  All rights reserved.
# $COPYRIGHT$
#
# Additional copyrights may follow
#
# $HEADER$
#

dist_opaldata_DATA = help-mpi-btl-tipc.txt

sources = \
    btl_tipc.c \
    btl_tipc.h \
    btl_tipc_addr.h \
    btl_tipc_component.c \
    btl_tipc_endpoint.c \
    btl_tipc_endpoint.h \
    btl_tipc_frag.c \
    btl_tipc_frag.h \
    btl_tipc_hdr.h \
    btl_tipc_proc.c \
    btl_tipc_proc.h

# Make the output library in this directory, and name it either
# mca_<type>_<name>.la (for DSO builds) or libmca_<type>_<name>.la
# (for static builds).

if MCA_BUILD_opal_btl_tipc_DSO
lib =
lib_sources =
component = mca_btl_tipc.la
component_sources = $(sources)
else
lib = libmca_btl_tipc.la
lib_sources = $(sources)
component =
component_sources =
endif

mcacomponentdir = $(opallibdir)
mcacomponent_LTLIBRARIES = $(component)
mca_btl_tipc_la_SOURCES = $(component_sources)
mca_btl_tipc_la_LDFLAGS = -module -avoid-version
if OPAL_cuda_support
mca_btl_tipc_la_LIBADD = $(top_builddir)/opal/lib@OPAL_LIB_NAME@.la \
    $(OPAL_TOP_BUILDDIR)/opal/mca/common/cuda/lib@OPAL_LIB_NAME@mca_common_cuda.la
endif

noinst_LTLIBRARIES = $(lib)
libmca_btl_tipc_la_SOURCES = $(lib_sources)
libmca_btl_tipc_la_LDFLAGS = -module -avoid-version
