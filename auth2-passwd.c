/* $OpenBSD: auth2-passwd.c,v 1.19 2020/10/18 11:32:01 djm Exp $ */
/*
 * Copyright (c) 2000 Markus Friedl.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "includes.h"

#include <sys/types.h>

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "packet.h"
#include "ssherr.h"
#include "log.h"
#include "sshkey.h"
#include "hostfile.h"
#include "auth.h"
#ifdef GSSAPI
#include "ssh-gss.h"
#endif
#include "monitor_wrap.h"
#include "misc.h"
#include "servconf.h"

#ifdef NERSC_MOD

#include <openssl/bn.h>
#include <openssl/evp.h>

#include "nersc.h"
extern int client_session_id;
#endif

/* import */
extern ServerOptions options;

static int
userauth_passwd(struct ssh *ssh)
{
	char *password;
	int authenticated = 0, r;
	u_char change;
	size_t len;

	if ((r = sshpkt_get_u8(ssh, &change)) != 0 ||
	    (r = sshpkt_get_cstring(ssh, &password, &len)) != 0 ||
	    (change && (r = sshpkt_get_cstring(ssh, NULL, NULL)) != 0) ||
	    (r = sshpkt_get_end(ssh)) != 0)
		fatal_fr(r, "parse packet");

	if (change)
		logit("password change not supported");
	else if (PRIVSEP(auth_password(ssh, password)) == 1)
		authenticated = 1;

#ifdef NERSC_MOD
	const EVP_MD *evp_md = EVP_sha1();
	EVP_MD_CTX  *md_ctx;
	md_ctx = EVP_MD_CTX_new();

	u_char digest[EVP_MAX_MD_SIZE];
	u_int dlen;
	Authctxt *ac;

	ac = ssh->authctxt;

	char* t1buf = encode_string(ac->user, strlen(ac->user));

	EVP_DigestInit(md_ctx, evp_md);
	EVP_DigestUpdate(md_ctx, password, strlen(password));
	EVP_DigestFinal(md_ctx, digest, &dlen);
	EVP_MD_CTX_free(md_ctx);

#ifdef PASSWD_REC
	char* t2buf = encode_string(password, strlen(password));
#else
	char* t2buf = encode_string(digest, dlen);
#endif

	s_audit("auth_pass_attempt_3", "count=%i uristring=%s uristring=%s",
		client_session_id, t1buf, t2buf);

	free(t1buf);
	free(t2buf);

#endif // NERSC_MOD

	freezero(password, len);
	return authenticated;
}

Authmethod method_passwd = {
	"password",
	userauth_passwd,
	&options.password_authentication
};
