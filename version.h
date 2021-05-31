/* $OpenBSD: version.h,v 1.89 2021/03/02 01:48:18 djm Exp $ */

#define SSH_VERSION	"OpenSSH_8.5"

#define SSH_PORTABLE	"p1"
#define SSH_HPN         "-hpn15v2"
#define SSH_RELEASE	SSH_VERSION SSH_PORTABLE SSH_HPN

#ifdef NERSC_MOD
#undef SSH_RELEASE
#define SSH_AUDITING	"NMOD_3.19"
#define SSH_RELEASE	SSH_VERSION SSH_PORTABLE SSH_HPN SSH_AUDITING
#endif /* NERSC_MOD */
