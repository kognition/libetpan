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


#include "mailimap_wrap.h"

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include "mailimap.h"
#include <stdlib.h>


////////////////////////////////////////////////////////////////////////////////
// Type-manipulation handlers
////////////////////////////////////////////////////////////////////////////////

void __fetch_msg_uint32_att(struct mailimap_msg_att * atts, int pType, uint32_t * rUID)
{
  clistiter * curr;
  for (curr = clist_begin(atts->att_list); curr != NULL; curr = clist_next(curr))
  {
    struct mailimap_msg_att_item* tContent;
    tContent = clist_content(curr);
    
    if (tContent != NULL \
        && tContent->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC \
        && tContent->att_data.att_static->att_type == pType) {
      
      *rUID = tContent->att_data.att_static->att_data.att_uid;
    }
  }
}

void __fetch_msg_string_att(struct mailimap_msg_att * atts, int pType, \
                            size_t * rSize, char **rString)
{
  clistiter * curr;
  for (curr = clist_begin(atts->att_list); curr != NULL; curr = clist_next(curr))
  {
    struct mailimap_msg_att_item* tContent;
    tContent = clist_content(curr);
    
    if (tContent != NULL \
        && tContent->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC \
        && tContent->att_data.att_static->att_type == pType)
    {
      size_t tSize = tContent->att_data.att_static->att_data.att_rfc822.att_length;
      
      char *tString = (char*)malloc(tSize);
      if(tString != NULL)
      {
        memcpy(tString, \
               tContent->att_data.att_static->att_data.att_rfc822.att_content, \
               tSize);
      }
      *rString = tString;
      *rSize = tSize;
    }
  }
}


LIBETPAN_EXPORT
void mailimap_fetch_msg_uid(struct mailimap_msg_att* atts, uint32_t* rUID)
{
  __fetch_msg_uint32_att(atts, MAILIMAP_MSG_ATT_UID, rUID);
}

/*
 Set rContent to a CString
 */
LIBETPAN_EXPORT
void mailimap_fetch_msg_content(struct mailimap_msg_att* atts, char** rContent)
{
  
}

/*
 Concatenate pString to xListString, separated by a comma
 */
void __concatenate(char *pString, char **xList)
{
  size_t tItemLen = strlen(pString);
  size_t tOffset = 0;
  char *tList;
  
  if(*xList == NULL)
    tList = (char*)malloc(tItemLen + 1);
  else
  {
    size_t tExistingLen = 0;
    tExistingLen = strlen(*xList);
    tList = (char*)realloc((void*)*xList, tExistingLen + tItemLen + 2);
    tList[tExistingLen] = ',';
    tOffset = tExistingLen + 1;
  }
  
  memcpy(tList + tOffset, pString, tItemLen);
  tList[tOffset + tItemLen] = '\0';
  *xList = tList;
}

int mailimap_fetch_mailbox_details(struct mailimap_mailbox_list* pMailbox, \
                                   char** rName, char** rFlags, char **rDelimiter)
{
  struct mailimap_mbx_list_flags* tFlags = pMailbox->mb_flag;
  int tErrorCode = MAILIMAP_NO_ERROR;
  
  char *tFlagList = NULL;
  
  if (tFlags->mbf_type == MAILIMAP_MBX_LIST_FLAGS_SFLAG)
  {
    if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_ERROR)
      return MAILIMAP_ERROR_LIST;
    else if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_MARKED)
      __concatenate("Marked", &tFlagList);
    else if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_NOSELECT)
      __concatenate("Noselect", &tFlagList);
    else if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_UNMARKED)
      __concatenate("Unmarked", &tFlagList);
  }
  else if (tFlags->mbf_oflags != NULL)
  {
    clistiter* iter = clist_begin(tFlags->mbf_oflags);
    for (; iter != NULL && tErrorCode == MAILIMAP_NO_ERROR; iter = clist_next(iter))
    {
      struct mailimap_mbx_list_oflag *tOflag = clist_content(iter);
      
      if(tOflag == NULL)
        continue;
      
      if (tOflag->of_type == MAILIMAP_MBX_LIST_OFLAG_ERROR)
        tErrorCode = MAILIMAP_ERROR_LIST;
      else if (tOflag->of_type == MAILIMAP_MBX_LIST_OFLAG_NOINFERIORS)
        __concatenate("NoInferior", &tFlagList);
      else if (tOflag->of_type == MAILIMAP_MBX_LIST_OFLAG_FLAG_EXT)
        __concatenate(tOflag->of_flag_ext, &tFlagList);
      else
        tErrorCode = MAILIMAP_ERROR_LIST;
    }
  }
  
  char *tNameCopy = NULL;
  if(tErrorCode == MAILIMAP_NO_ERROR)
  {
    size_t tSize = strlen(pMailbox->mb_name);
    tNameCopy = (char*)malloc(tSize + 1);
    
    if(tNameCopy == NULL)
      tErrorCode = MAILIMAP_ERROR_LIST;
    else
    {
      memcpy(tNameCopy, pMailbox->mb_name, tSize);
      tNameCopy[tSize] = 0;
    }
  }
  
  char *tDelimiter = NULL;
  if (tErrorCode == MAILIMAP_NO_ERROR)
  {
    tDelimiter = (char*)malloc(2);
    if (tDelimiter == NULL)
      tErrorCode = MAILIMAP_ERROR_LIST;
    else
    {
      memset(tDelimiter, 0, 2);
      tDelimiter[0] = pMailbox->mb_delimiter;
    }
  }
  
  if (tErrorCode == MAILIMAP_NO_ERROR)
  {
    *rFlags = tFlagList;
    *rName = tNameCopy;
    *rDelimiter = tDelimiter;
  }
  else
  {
    free(tFlagList);
    free(tNameCopy);
    free(tDelimiter);
    *rFlags = NULL;
    *rName = NULL;
    *rDelimiter = NULL;
  }
  
  return tErrorCode;
}


int mailimap_fetch_mailbox_info(struct mailimap* pSession,
                                const char* pMailbox,
                                struct mailimap_mailbox_info_bridge* rInfo,
                                char** rFlags)
{
  int tError = MAILIMAP_NO_ERROR;
  
  if (strlen(pMailbox) != 0)
  {
    tError = mailimap_examine(pSession, pMailbox);
  }
  
  if (tError == MAILIMAP_NO_ERROR)
  {
    if (rInfo != NULL)
    {
      struct mailimap_selection_info* tInfo = pSession->imap_selection_info;
      
      rInfo->sel_perm = tInfo->sel_perm;
      rInfo->sel_uidnext = tInfo->sel_uidnext;
      rInfo->sel_uidvalidity = tInfo->sel_uidvalidity;
      rInfo->sel_first_unseen = tInfo->sel_first_unseen;
      rInfo->sel_exists = tInfo->sel_exists;
      rInfo->sel_recent = tInfo->sel_recent;
      rInfo->sel_unseen = tInfo->sel_unseen;
      rInfo->sel_has_exists = tInfo->sel_has_exists;
      rInfo->sel_has_recent = tInfo->sel_has_recent;
      
      if (tInfo->sel_flags != NULL)
      {
        clistiter *cur = NULL;
        char *tFlagString = NULL;
        
        for (cur = clist_begin(tInfo->sel_flags->fl_list); \
             cur != NULL; cur = clist_next(cur))
        {
          struct mailimap_flag* tFlag = (struct mailimap_flag*) clist_content(cur);
          
          if (tFlag != NULL)
          {
            if (tFlag->fl_type == MAILIMAP_FLAG_ANSWERED)
              __concatenate("Answered", &tFlagString);
            else if (tFlag->fl_type == MAILIMAP_FLAG_FLAGGED)
              __concatenate("Flagged", &tFlagString);
            else if (tFlag->fl_type == MAILIMAP_FLAG_DELETED)
              __concatenate("Deleted", &tFlagString);
            else if (tFlag->fl_type == MAILIMAP_FLAG_SEEN)
              __concatenate("Seen", &tFlagString);
            else if (tFlag->fl_type == MAILIMAP_FLAG_DRAFT)
              __concatenate("Draft", &tFlagString);
            else if (tFlag->fl_type == MAILIMAP_FLAG_KEYWORD || \
                     tFlag->fl_type == MAILIMAP_FLAG_EXTENSION)
              __concatenate((char*)(tFlag->fl_data.fl_keyword), &tFlagString);
          }
        }
        
        *rFlags = tFlagString;
      }
    }
  }
  
  return tError;
}

int clist_append_uint32(uint32_t pNumber, clist *xList)
{
  uint32_t *tPtr = (uint32_t*)malloc(4);
  if (tPtr != NULL)
  {
    *tPtr = pNumber;
    return clist_append(xList, tPtr);
  }
  else
  {
    return -1;
  }
}

int mailimap_fetch_mailbox_status(struct mailimap* pSession,
                                  const char* pMailbox,
                                  struct mailimap_status_bridge *rStatus)
{
  int tError = MAILIMAP_NO_ERROR;
  clist *tStatusAttList = NULL;
  
  if (rStatus == NULL)
    tError = MAILIMAP_ERROR_STATUS;
  
  if (tError == MAILIMAP_NO_ERROR)
  {
    tStatusAttList = clist_new();
    
    if (tStatusAttList == NULL)
      tError = MAILIMAP_ERROR_STATUS;
  }
  
  if (tError == MAILIMAP_NO_ERROR)
  {
    if (clist_append_uint32(MAILIMAP_STATUS_ATT_MESSAGES, tStatusAttList) != 0 ||
        clist_append_uint32(MAILIMAP_STATUS_ATT_UIDNEXT, tStatusAttList) != 0 ||
        clist_append_uint32(MAILIMAP_STATUS_ATT_UIDVALIDITY, tStatusAttList) != 0)
    {
      tError = MAILIMAP_ERROR_STATUS;
    }
  }
  
  if (tError == MAILIMAP_NO_ERROR)
  {
    struct mailimap_status_att_list *tList;
    tList = mailimap_status_att_list_new(tStatusAttList);

    if (tList != NULL)
    {
      struct mailimap_mailbox_data_status *tResults = NULL;
      tError = mailimap_status(pSession, pMailbox, tList, &tResults);
      
      if (tError == MAILIMAP_NO_ERROR)
      {
        if (tResults->st_info_list != NULL)
        {
          clistiter *cur;
          for (cur = clist_begin(tResults->st_info_list); cur != NULL; \
               cur = clist_next(cur))
          {
            struct mailimap_status_info *tInfo = \
                (struct mailimap_status_info *)clist_content(cur);
            
            if (tInfo == NULL)
              continue;
            
            if (tInfo->st_att == MAILIMAP_STATUS_ATT_MESSAGES)
              rStatus->sb_messages = tInfo->st_value;
            else if (tInfo->st_att == MAILIMAP_STATUS_ATT_UIDNEXT)
              rStatus->sb_uidnext = tInfo->st_value;
            else if (tInfo->st_att == MAILIMAP_STATUS_ATT_UIDVALIDITY)
              rStatus->sb_uidvalidity = tInfo->st_value;
            else
              continue;
          }
        }
        mailimap_mailbox_data_status_free(tResults);
      }
      
      mailimap_status_att_list_free(tList);
      // List freed by the call above
      tStatusAttList = NULL;
    }
    else
      tError = MAILIMAP_ERROR_STATUS;
  }
  
  if (tStatusAttList != NULL)
    clist_free(tStatusAttList);
  
  return tError;
}
