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
#include <stdio.h>


////////////////////////////////////////////////////////////////////////////////
// Type-manipulation handlers
////////////////////////////////////////////////////////////////////////////////

void __fetch_msg_uint32_att(struct mailimap_msg_att *pAtts, int pType, uint32_t * rUID)
{
  clistiter *tCur;
  if (pAtts == NULL)
    return;
  
  for (tCur = clist_begin(pAtts->att_list); tCur != NULL; tCur = clist_next(tCur))
  {
    struct mailimap_msg_att_item* tContent;
    tContent = clist_content(tCur);
    
    if (tContent != NULL
        && tContent->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC
        && tContent->att_data.att_static->att_type == pType) {
      
      *rUID = tContent->att_data.att_static->att_data.att_uid;
    }
  }
}

void __fetch_msg_string_att(struct mailimap_msg_att *pAtts, int pType,
                            size_t * rSize, char **rString)
{
  clistiter *tCur;
  if (pAtts == NULL)
    return;
  for (tCur = clist_begin(pAtts->att_list); tCur != NULL; tCur = clist_next(tCur))
  {
    struct mailimap_msg_att_item* tContent;
    tContent = clist_content(tCur);
    
    if (tContent != NULL
        && tContent->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC
        && tContent->att_data.att_static->att_type == pType)
    {
      size_t tSize = tContent->att_data.att_static->att_data.att_rfc822.att_length;
      
      char *tString = (char*)malloc(tSize);
      if(tString != NULL)
      {
        memcpy(tString,
               tContent->att_data.att_static->att_data.att_rfc822.att_content,
               tSize);
      }
      *rString = tString;
      *rSize = tSize;
    }
  }
}

void __concatWithDel(char *pString, const char *pDel, char **xList)
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
    size_t tDelLen = strlen(pDel);
    
    tList = (char*)realloc((void*)*xList, tExistingLen + tItemLen + tDelLen + 1);
    
    if (tDelLen > 0)
    {
      strcpy(&(tList[tExistingLen]), pDel);
    }
    tOffset = tExistingLen + tDelLen;
  }
  
  memcpy(tList + tOffset, pString, tItemLen);
  tList[tOffset + tItemLen] = '\0';
  *xList = tList;
}


/*
 Concatenate pString to xListString, separated by a comma
 */
void __concatToList(char *pString, char **xList)
{
  __concatWithDel(pString, ",", xList);
}

void __concat(char *pString, char **xString)
{
  __concatWithDel(pString, "", xString);
}


char *__uintListToCString(clist *pList)
{
  clistiter *tCur;
  size_t tStringLength = 0;
  char *tStringList = NULL;
  
  if (pList == NULL)
    return tStringList;
  
  for (tCur = clist_begin(pList); tCur != NULL; tCur = clist_next(tCur))
  {
    uint32_t *tContent = (uint32_t*)clist_content(tCur);
    int tContentLength = snprintf(NULL, 0,"%u",*tContent);
    
    size_t tNewLength;
    tNewLength = tStringLength + tContentLength;
    
    // Add one extra char for a comma
    if (tStringList != NULL)
      tNewLength++;
    
    tStringList = (char*)realloc(tStringList, tNewLength + 1);
    
    if (tStringList == NULL)
      continue;
    
    // Append a comma at the end of the previous string
    char *tDest = tStringList;
    if (tStringLength != 0)
    {
      tStringList[tStringLength] = ',';
      tDest = (char *)(tStringList + tStringLength + 1);
    }
    
    int tWritten = snprintf(tDest, tContentLength + 1, "%u", *tContent);
    
    tStringLength = tNewLength;
    if (tWritten != tContentLength)
      continue;
  }
  
  return tStringList;
}

/*
 * Concatenate the flag to the CString xString
 */
void __concatenateFlags(struct mailimap_flag *pFlag, char **xString)
{
  if (pFlag != NULL)
  {
    if (pFlag->fl_type == MAILIMAP_FLAG_ANSWERED)
      __concatToList("Answered", xString);
    else if (pFlag->fl_type == MAILIMAP_FLAG_FLAGGED)
      __concatToList("Flagged", xString);
    else if (pFlag->fl_type == MAILIMAP_FLAG_DELETED)
      __concatToList("Deleted", xString);
    else if (pFlag->fl_type == MAILIMAP_FLAG_SEEN)
      __concatToList("Seen", xString);
    else if (pFlag->fl_type == MAILIMAP_FLAG_DRAFT)
      __concatToList("Draft", xString);
    else if (pFlag->fl_type == MAILIMAP_FLAG_KEYWORD ||
             pFlag->fl_type == MAILIMAP_FLAG_EXTENSION)
      __concatToList((char*)(pFlag->fl_data.fl_keyword), xString);
  }
}


//////////
// Exported functions
//////////

typedef struct error_label
{
  int el_enum;
  const char *el_label;
} error_label;

const error_label error_name_map[] =
{
  {MAILIMAP_NO_ERROR, "no error"},
  {MAILIMAP_NO_ERROR_AUTHENTICATED, "no error - authenticated"},
  {MAILIMAP_NO_ERROR_NON_AUTHENTICATED, "no error - non authenticated"},
  {MAILIMAP_ERROR_BAD_STATE, "bad state"},
  {MAILIMAP_ERROR_STREAM, "stream error"},
  {MAILIMAP_ERROR_PARSE, "parse error"},
  {MAILIMAP_ERROR_CONNECTION_REFUSED, "connection refused"},
  {MAILIMAP_ERROR_MEMORY, "memory error"},
  {MAILIMAP_ERROR_FATAL, "fatal error"},
  {MAILIMAP_ERROR_PROTOCOL, "protocol error"},
  {MAILIMAP_ERROR_DONT_ACCEPT_CONNECTION, "don't accept connection"},
  {MAILIMAP_ERROR_APPEND, "command APPEND error"},
  {MAILIMAP_ERROR_NOOP, "command NOOP"},
  {MAILIMAP_ERROR_LOGOUT, "command LOGOUT error"},
  {MAILIMAP_ERROR_CAPABILITY, "command CAPABILITY error"},
  {MAILIMAP_ERROR_CHECK, "check error"},
  {MAILIMAP_ERROR_CLOSE, "command CLOSE error"},
  {MAILIMAP_ERROR_EXPUNGE, "command EXPUNGE error"},
  {MAILIMAP_ERROR_COPY, "command COPY error"},
  {MAILIMAP_ERROR_UID_COPY, "command UID COPY error"},
  {MAILIMAP_ERROR_MOVE, "command MOVE error"},
  {MAILIMAP_ERROR_UID_MOVE, "command UID MOVE error"},
  {MAILIMAP_ERROR_CREATE, "command CREATE error"},
  {MAILIMAP_ERROR_DELETE, "command DELETE error"},
  {MAILIMAP_ERROR_EXAMINE, "command EXAMINE error"},
  {MAILIMAP_ERROR_FETCH, "command FETCH error"},
  {MAILIMAP_ERROR_UID_FETCH, "command UID FETCH error"},
  {MAILIMAP_ERROR_LIST, "command LIST error"},
  {MAILIMAP_ERROR_LOGIN, "command LOGIN error"},
  {MAILIMAP_ERROR_LSUB, "command LSUB error"},
  {MAILIMAP_ERROR_RENAME, "command RENAME error"},
  {MAILIMAP_ERROR_SEARCH, "command SEARCH error"},
  {MAILIMAP_ERROR_UID_SEARCH, "command UID SEARCH error"},
  {MAILIMAP_ERROR_SELECT, "command SELECT error"},
  {MAILIMAP_ERROR_STATUS, "command STATUS error"},
  {MAILIMAP_ERROR_STORE, "command STORE error"},
  {MAILIMAP_ERROR_UID_STORE, "command UID STORE error"},
  {MAILIMAP_ERROR_SUBSCRIBE, "command SUBSCRIBE error"},
  {MAILIMAP_ERROR_UNSUBSCRIBE, "command UNSUBSCRIBE error"},
  {MAILIMAP_ERROR_STARTTLS, "starttls error"},
  {MAILIMAP_ERROR_INVAL, "inval error"},
  {MAILIMAP_ERROR_EXTENSION, "extension error"},
  {MAILIMAP_ERROR_SASL, "sasl error"},
  {MAILIMAP_ERROR_SSL, "ssl error"},
  {MAILIMAP_ERROR_NEEDS_MORE_DATA, "needs more data"},
  {MAILIMAP_ERROR_CUSTOM_COMMAND, "custom command error"},
  {MAILIMAP_ERROR_CLIENTID, "client ID error"}
};

LIBETPAN_EXPORT
void mailimap_get_error_label(int pErrorCode, char **rLabel)
{
  uint32_t tSize = sizeof(error_name_map) / sizeof(error_name_map[0]);
  char *tLabel = NULL;
  
  for (uint32_t i = 0; i < tSize; i++)
  {
    if (error_name_map[i].el_enum == pErrorCode)
      tLabel = strdup(error_name_map[i].el_label);
  }
  
  if (tLabel == NULL)
    tLabel = strdup("cannot find error code");
  
  *rLabel = tLabel;
}

typedef struct search_flag
{
  const char *sf_name;
  int sf_value;
} search_flag;

const search_flag search_flag_map[] =
{
  {"ANSWERED", MAILIMAP_SEARCH_KEY_ANSWERED},
  {"DELETED", MAILIMAP_SEARCH_KEY_DELETED},
  {"DRAFT", MAILIMAP_SEARCH_KEY_DRAFT},
  {"FLAGGED", MAILIMAP_SEARCH_KEY_FLAGGED},
  {"NEW", MAILIMAP_SEARCH_KEY_NEW},
  {"RECENT", MAILIMAP_SEARCH_KEY_RECENT},
  {"OLD", MAILIMAP_SEARCH_KEY_OLD},
  {"SEEN", MAILIMAP_SEARCH_KEY_SEEN},
  {"UNANSWERED", MAILIMAP_SEARCH_KEY_UNANSWERED},
  {"UNDELETED", MAILIMAP_SEARCH_KEY_UNDELETED},
  {"UNDRAFT", MAILIMAP_SEARCH_KEY_UNDRAFT},
  {"UNFLAGGED", MAILIMAP_SEARCH_KEY_UNFLAGGED},
  {"UNSEEN", MAILIMAP_SEARCH_KEY_UNSEEN}
};

LIBETPAN_EXPORT
struct mailimap_search_key *
mailimap_search_key_new_flag(const char *flag)
{
  uint32_t tElements = sizeof(search_flag_map) / sizeof(search_flag_map[0]);
  
  int tType = -1;
  for (uint32_t i = 0; i < tElements && tType == -1; i++)
  {
    if (strcmp(search_flag_map[i].sf_name, flag) == 0)
      tType = search_flag_map[i].sf_value;
  }
  
  if (tType != -1)
  {
    return mailimap_search_key_new(tType, NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL, NULL,
                                   NULL, NULL, NULL, NULL, NULL,
                                   NULL, 0, NULL,
                                   NULL, NULL, NULL, NULL, NULL,
                                   0, NULL, NULL, NULL);
  }
  
  return NULL;
}

int mailimap_fetch_mailbox_details(struct mailimap_mailbox_list *pMailbox,
                                   char **rName, char **rFlags, char **rDelimiter)
{
  struct mailimap_mbx_list_flags* tFlags = pMailbox->mb_flag;
  int tErrorCode = MAILIMAP_NO_ERROR;
  
  char *tFlagList = NULL;
  
  if (tFlags->mbf_type == MAILIMAP_MBX_LIST_FLAGS_SFLAG)
  {
    if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_ERROR)
      return MAILIMAP_ERROR_LIST;
    else if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_MARKED)
      __concatToList("Marked", &tFlagList);
    else if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_NOSELECT)
      __concatToList("Noselect", &tFlagList);
    else if (tFlags->mbf_sflag == MAILIMAP_MBX_LIST_SFLAG_UNMARKED)
      __concatToList("Unmarked", &tFlagList);
  }
  
  if (tFlags->mbf_oflags != NULL)
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
        __concatToList("NoInferior", &tFlagList);
      else if (tOflag->of_type == MAILIMAP_MBX_LIST_OFLAG_FLAG_EXT)
        __concatToList(tOflag->of_flag_ext, &tFlagList);
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


int mailimap_fetch_mailbox_info(struct mailimap *pSession,
                                const char *pMailbox,
                                struct mailimap_mailbox_info_bridge *rInfo,
                                char **rFlags)
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
        clistiter *cur = clist_begin(tInfo->sel_flags->fl_list);
        char *tFlagString = NULL;
        
        for (; cur != NULL; cur = clist_next(cur))
        {
          __concatenateFlags((struct mailimap_flag*) clist_content(cur),
                            &tFlagString);
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
          clistiter *cur = clist_begin(tResults->st_info_list);
          for (; cur != NULL; cur = clist_next(cur))
          {
            struct mailimap_status_info *tInfo =
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


int mailimap_run_search(struct mailimap *pSession,
                        struct mailimap_search_key *pSearchKeys,
                        char **rList)
{
  clist *tResults = NULL;
  int tCode;
  char *tStringList = NULL;
  
  tCode = mailimap_search(pSession, "UTF-8", pSearchKeys, &tResults);
  
  if (tCode == MAILIMAP_NO_ERROR)
  {
    tStringList = __uintListToCString(tResults);
    clist_free(tResults);
  }
  
  *rList = tStringList;
  return tCode;
}


int mailimap_run_uid_search(struct mailimap *pSession,
                            struct mailimap_search_key *pSearchKeys,
                            char **rList)
{
  clist *tResults = NULL;
  int tCode;
  char *tStringList = NULL;
  
  tCode = mailimap_uid_search(pSession, "UTF-8", pSearchKeys, &tResults);
  
  if (tCode == MAILIMAP_NO_ERROR)
  {
    tStringList = __uintListToCString(tResults);
    clist_free(tResults);
  }
  
  *rList = tStringList;
  return tCode;
}


void __extractEmailInfo(struct mailimap_msg_att * pMsgAtt,
                        struct mailimap_email_details* xDetails,
                        char **xError)
{
  clistiter *cur = clist_begin(pMsgAtt->att_list);
  for (; cur != NULL; cur = clist_next(cur))
  {
    struct mailimap_msg_att_item *tItem = (struct mailimap_msg_att_item*) clist_content(cur);
    
    if (tItem->att_type == MAILIMAP_MSG_ATT_ITEM_ERROR)
    {
      __concatWithDel("parse error!", "\n", xError);
    }
    else if (tItem->att_type == MAILIMAP_MSG_ATT_ITEM_STATIC)
    {
      struct mailimap_msg_att_static *tAtt = tItem->att_data.att_static;
      
      if (tAtt->att_type == MAILIMAP_MSG_ATT_ENVELOPE)
        xDetails->ed_subject = tAtt->att_data.att_env->env_subject;
      else if (tAtt->att_type == MAILIMAP_MSG_ATT_RFC822_SIZE)
        xDetails->ed_size = tAtt->att_data.att_rfc822_size;
      else if (tAtt->att_type == MAILIMAP_MSG_ATT_INTERNALDATE)
        xDetails->ed_internalDate = tAtt->att_data.att_internal_date;
      else if (tAtt->att_type == MAILIMAP_MSG_ATT_UID)
        xDetails->ed_uid = tAtt->att_data.att_uid;
      else if (tAtt->att_type == MAILIMAP_MSG_ATT_BODY_SECTION)
      {
        xDetails->ed_body.ed_size = tAtt->att_data.att_body_section->sec_length;
        xDetails->ed_body.ed_content =  tAtt->att_data.att_body_section->sec_body_part;
      }
      else
      {
        __concatWithDel("Non-requested static type: ", "\n", xError);
        char tCode[3];
        if (snprintf(tCode, 3, "%i", tAtt->att_type) != 3)
          tCode[2] = '\0';
        __concat(tCode, xError);
      }
    }
    else if (tItem->att_type == MAILIMAP_MSG_ATT_ITEM_DYNAMIC)
    {
      __concatWithDel("Non requested dynamic att", "\n", xError);
    }
    else if (tItem->att_type == MAILIMAP_MSG_ATT_ITEM_EXTENSION)
    {
      __concatWithDel("This is external stuff, nope!", "\n", xError);
    }
    else
    {
      __concatWithDel("That shouldn't happen!", "\n", xError);
    }
  }
}

void __msg_att_handler(struct mailimap_msg_att * pMsgAtt, void * pContext)
{
  struct mailimap_msg_att_fetch_callback *tCallback =
                          (struct mailimap_msg_att_fetch_callback*)pContext;
  
  if (tCallback == NULL)
    return;
  
  clistiter *cur = NULL;
  
  if (pMsgAtt != NULL)
  {
    cur = clist_begin(pMsgAtt->att_list);
  }
  
  char *tError = NULL;
  struct mailimap_email_details tDetails;
  tDetails.ed_body.ed_content = NULL;
  tDetails.ed_subject = NULL;
  tDetails.ed_size = 0;
  tDetails.ed_uid = 0;
  tDetails.ed_internalDate = NULL;
  
  
  if (tCallback->fc_type == MAILIMAP_MSG_ATT_CALLBACK_FETCH_EMAIL_DETAILS)
  {
    char *tDateItems = NULL;
    __extractEmailInfo(pMsgAtt, &tDetails, &tError);
    
    if (tDetails.ed_internalDate != NULL)
    {
      int tDateLength;
      tDateLength = snprintf(NULL, 0, "%i,%i,%i,%i,%i,%i,%i",
                             tDetails.ed_internalDate->dt_year,
                             tDetails.ed_internalDate->dt_month,
                             tDetails.ed_internalDate->dt_day,
                             tDetails.ed_internalDate->dt_hour,
                             tDetails.ed_internalDate->dt_min,
                             tDetails.ed_internalDate->dt_sec,
                             tDetails.ed_internalDate->dt_zone);
      
      if (tDateLength > 0)
      {
        tDateItems = (char*)malloc(tDateLength + 1);
      }
    
      if (tDateItems != NULL)
      {
        if (snprintf(tDateItems, tDateLength + 1, "%i,%i,%i,%i,%i,%i,%i",
                     tDetails.ed_internalDate->dt_year,
                     tDetails.ed_internalDate->dt_month,
                     tDetails.ed_internalDate->dt_day,
                     tDetails.ed_internalDate->dt_hour,
                     tDetails.ed_internalDate->dt_min,
                     tDetails.ed_internalDate->dt_sec,
                     tDetails.ed_internalDate->dt_zone) != tDateLength)
        {
          __concatWithDel("error in formatting internal date", "\n", &tError);
        }
      }
    }
    else
    {
      __concatWithDel("error: no internal date returned", "\n", &tError);
    }
    
    tCallback->fc_callback.fc_email_details(tDetails.ed_uid, tDetails.ed_size,
                                    tDetails.ed_subject, tDateItems, tError,
                                    *(uint32_t*)(tCallback->fc_context));
    
    free(tDateItems);
  }
  else if (tCallback->fc_type == MAILIMAP_MSG_ATT_CALLBACK_FETCH_EMAIL_BODY)
  {
    __extractEmailInfo(pMsgAtt, &tDetails, &tError);
    
    tCallback->fc_callback.fc_email_body(tDetails.ed_uid,
                                         tDetails.ed_body.ed_size,
                                         tDetails.ed_body.ed_content,
                                         tError,
                                         *(uint32_t*)tCallback->fc_context);
  }
  
  free(tError);
}


void progress_fun(size_t current, size_t maximum, void * context)
{
  fprintf(stdout, "item %zu/%zu\n", current, maximum);
}


int __add_att_list(struct mailimap_fetch_type **xFetchType,
                   struct mailimap_fetch_att *(new_att)(void))
{
  int tCode = MAILIMAP_NO_ERROR;
  struct mailimap_fetch_type* tFetchType = NULL;
  
  if (*xFetchType == NULL)
  {
    tFetchType = mailimap_fetch_type_new_fetch_att_list_empty();
    
    if (tFetchType == NULL)
      tCode = MAILIMAP_ERROR_MEMORY;
    else
      *xFetchType = tFetchType;
  }
  else
  {
    tFetchType = *xFetchType;
  }
  
  if (tFetchType != NULL)
  {
    struct mailimap_fetch_att* tAtt;
    tAtt = new_att();
    
    if (tAtt == NULL)
    {
      tCode = MAILIMAP_ERROR_MEMORY;
    }
    else
    {
      tCode = mailimap_fetch_type_new_fetch_att_list_add(tFetchType, tAtt);
    }
  }
  
  return tCode;
}


int mailimap_fetch_email_details(mailimap * pSession,
                                 struct mailimap_set *pSet,
                                 fetch_email_details_callback *pCallback,
                                 uint32_t pID)
{
  int tCode;
  clist *tFetchList = NULL;
  
  struct mailimap_email_details tEmailInfo;
  tEmailInfo.ed_subject = NULL;
  tEmailInfo.ed_internalDate = NULL;
  tEmailInfo.ed_size = 0;
  
  struct mailimap_msg_att_fetch_callback tCallback;
  tCallback.fc_type = MAILIMAP_MSG_ATT_CALLBACK_FETCH_EMAIL_DETAILS;
  tCallback.fc_callback.fc_email_details = pCallback;
  tCallback.fc_context = &pID;
  
  // Even though unused, a progress callback function must be added too, so that
  // msg_att_handler gets called when the response is parsed
  mailimap_set_progress_callback(pSession, NULL, &progress_fun, NULL);
  mailimap_set_msg_att_handler(pSession, &__msg_att_handler, (void*)(&tCallback));
  
  struct mailimap_fetch_type *tFetchType = NULL;
  tCode = __add_att_list(&tFetchType, &mailimap_fetch_att_new_envelope);
  
  if (tCode == MAILIMAP_NO_ERROR)
    tCode = __add_att_list(&tFetchType, &mailimap_fetch_att_new_internaldate);
  
  if (tCode == MAILIMAP_NO_ERROR)
    tCode = __add_att_list(&tFetchType, &mailimap_fetch_att_new_rfc822_size);
  
  if (tCode == MAILIMAP_NO_ERROR)
    tCode = mailimap_uid_fetch(pSession, pSet, tFetchType, &tFetchList);
  
  mailimap_fetch_type_free(tFetchType);
  
  // Remove callback function pointers
  mailimap_set_progress_callback(pSession, NULL, NULL, NULL);
  mailimap_set_msg_att_handler(pSession, NULL, NULL);
  
  if (tFetchList != NULL)
    mailimap_fetch_list_free(tFetchList);
  
  return tCode;
}


int mailimap_fetch_email_body(mailimap *pSession,
                              struct mailimap_set *pSet,
                              fetch_email_body_callback *pCallback,
                              uint32_t pID)
{
  int tCode = MAILIMAP_NO_ERROR;
  
  struct mailimap_email_details tEmailInfo;
  tEmailInfo.ed_subject = NULL;
  tEmailInfo.ed_internalDate = NULL;
  tEmailInfo.ed_size = 0;
  
  struct mailimap_msg_att_fetch_callback tCallback;
  tCallback.fc_type = MAILIMAP_MSG_ATT_CALLBACK_FETCH_EMAIL_BODY;
  tCallback.fc_callback.fc_email_body = pCallback;
  tCallback.fc_context = &pID;
  
  // Even though unused, a progress callback function must be added too, so that
  // msg_att_handler gets called when the response is parsed
  mailimap_set_progress_callback(pSession, NULL, &progress_fun, NULL);
  mailimap_set_msg_att_handler(pSession, &__msg_att_handler, (void*)(&tCallback));
  
  struct mailimap_section* tSection = mailimap_section_new(NULL);
  if (tSection == NULL)
    return MAILIMAP_ERROR_MEMORY;
  
  struct mailimap_fetch_att *tFetchAtt = NULL;
  tFetchAtt = mailimap_fetch_att_new_body_peek_section(tSection);
  
  if (tFetchAtt == NULL)
  {
    mailimap_section_free(tSection);
    return MAILIMAP_ERROR_MEMORY;
  }
  
  struct mailimap_fetch_type *tFetchType = NULL;
  tFetchType = mailimap_fetch_type_new_fetch_att_list_empty();
  
  if (tFetchType == NULL)
  {
    mailimap_fetch_att_free(tFetchAtt);
    return MAILIMAP_ERROR_MEMORY;
  }
  
  tCode = mailimap_fetch_type_new_fetch_att_list_add(tFetchType, tFetchAtt);
  
  clist *tFetchList = NULL;
  if (tCode == MAILIMAP_NO_ERROR)
    tCode = mailimap_uid_fetch(pSession, pSet, tFetchType, &tFetchList);
  
  mailimap_fetch_type_free(tFetchType);
  
  // Remove callback function pointers
  mailimap_set_progress_callback(pSession, NULL, NULL, NULL);
  mailimap_set_msg_att_handler(pSession, NULL, NULL);
  
  if (tFetchList != NULL)
    mailimap_fetch_list_free(tFetchList);
  
  return tCode;
}
