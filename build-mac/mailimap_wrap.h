/*
 * libEtPan! -- a mail stuff library
 *
 * Copyright (C) 2001, 2005 - DINH Viet Hoa
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the libEtPan! project nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */


#ifndef mailimap_wrap_h
#define mailimap_wrap_h


#ifdef __cplusplus
extern "C" {
#endif
  
#include <stdio.h>

#include "mailimap.h"
  
struct mailimap_mailbox_info_bridge {
  int32_t sel_perm;
  uint32_t sel_uidnext;
  uint32_t sel_uidvalidity;
  uint32_t sel_first_unseen;
  uint32_t sel_exists;
  uint32_t sel_recent;
  uint32_t sel_unseen;
  uint8_t  sel_has_exists;
  uint8_t  sel_has_recent;
};

struct mailimap_status_bridge {
  uint32_t sb_messages;
  uint32_t sb_uidnext;
  uint32_t sb_uidvalidity;
};

/*
 mailimap_fetch_mailbox_details()
 This function fetches details of a mailbox obtained from mailimap_list
 
 @param rFlags      comma-separated list of flags
 @param rDelimiter  delimiter in the mailbox path
 @param rName       name of the mailbox
 
 @return the return code is one of MAILIMAP_NO_ERROR or MAILIMAP_ERROR_XXX
 */
LIBETPAN_EXPORT
int mailimap_fetch_mailbox_details(struct mailimap_mailbox_list* pMailbox,
                                     char** rName, char** rFlags, char **rDelimiter);

/*
 mailimap_fetch_mailbox_status()
 This function fetches the status of a mailbox
 
 @param pMailbox  targeted mailbox
 @param rMessages message count in that mailbox
 @param rUidNext  next UID for that mailbox
*/
LIBETPAN_EXPORT
int mailimap_fetch_mailbox_status(struct mailimap* pSession,
                                  const char* pMailbox,
                                  struct mailimap_status_bridge *rStatus);
  
/*
 mailimap_fetch_mailbox_info()
 This function examines pMailbox and returns its attributes
 
 @param pMailbox  Mailbox name. If empty, fetches info of the current mailbox
 @param rInfo     Selection info. Must not be freed
*/
int mailimap_fetch_mailbox_info(struct mailimap* pSession,
                                const char* pMailbox,
                                struct mailimap_mailbox_info_bridge* rInfo,
                                char **rFlags);

#ifdef __cplusplus
}
#endif
  
#endif /* mailimap_wrap_h */
