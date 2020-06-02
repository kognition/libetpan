//
//
//

#include <stdio.h>
#include "mailimap_wrap.h"

#include <libetpan/libetpan.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Test utilities
////////////////////////////////////////////////////////////////////////////////


void TestPrintResults(void);
bool TestAssert(const char* pMessage, bool pTest);

bool TestCompareInts(int pFound, int pExpected, const char* pMessage);
bool TestCompareIntsNot(int pFound, int pExpected, const char* pMessage);

uint32_t s_test_executed;
uint32_t s_test_passed;

void TestPrintResults()
{
  fprintf(stdout, "Test passed [%u/%u]\n", s_test_passed, s_test_executed);
}

void TestInitialise()
{
  s_test_executed = 0;
  s_test_passed = 0;
}

bool _TestProcess(const char *pMessage, bool pSuccess)
{
  bool t = false;
  s_test_executed++;
  
  const char* tSuccess;
  
  if (pSuccess)
  {
    t = true;
    tSuccess = "";
    s_test_passed++;
  }
  else
    tSuccess = "NOT ";
  
  fprintf(stdout, "%sOK - %s", tSuccess, pMessage);
  return t;
}

bool TestAssert(const char* pMessage, bool pTest)
{
  _TestProcess(pMessage, pTest);
  fputs("\n", stdout);
  
  return pTest;
}

bool _TestCompareStrings(const char *pFound, const char *pExpected,
                        const char *pMessage, bool pFail)
{
  int tCmp = strcmp(pFound, pExpected);
  bool tSuccess = _TestProcess(pMessage,
                               (!pFail && tCmp == 0) ||
                               (pFail && tCmp != 0));
  
  if (tSuccess)
  {
    if (!pFail)
      fprintf(stdout, ": %s", pFound);
    else
      fprintf(stdout, ": expected not %s, found %s", \
              pExpected, pFound);
  }
  else
  {
    if (!pFail)
      fprintf(stdout, ": expected %s, found %s", pExpected, pFound);
    else
      fprintf(stdout, ": expected not %s", pExpected);
  }
  fputs("\n", stdout);
  return tSuccess;
}

bool TestCompareStrings(const char *pFound, const char *pExpected,
                        const char *pMessage)
{
  return _TestCompareStrings(pFound, pExpected, pMessage, false);
}

bool TestCompareStringsNot(const char *pFound, const char *pExpected,
                           const char *pMessage)
{
  return _TestCompareStrings(pFound, pExpected, pMessage, true);
}

bool _TestCompareInts(int pFound, int pExpected, const char* pMessage, bool pFail)
{
  bool tSuccess = _TestProcess(pMessage,
                              (!pFail && pFound == pExpected) ||
                               (pFail && pFound != pExpected));

  if (tSuccess)
  {
    if (!pFail)
      fprintf(stdout, ": %i", pFound);
    else
      fprintf(stdout, ": expected not %i, found %i", \
            pExpected, pFound);
  }
  else
  {
    if (!pFail)
      fprintf(stdout, ": expected %i, found %i", pExpected, pFound);
    else
      fprintf(stdout, ": expected not %i", pExpected);
  }
  fputs("\n", stdout);
  return tSuccess;
}

bool TestCompareInts(int pFound, int pExpected, const char* pMessage)
{
  return _TestCompareInts(pFound, pExpected, pMessage, false);
}

bool TestCompareIntsNot(int pFound, int pExpected, const char* pMessage)
{
  return _TestCompareInts(pFound, pExpected, pMessage, true);
}

bool _TestCompareUInts(uint32_t pFound, uint32_t pExp, const char* pMessage, bool pFail)
{
  bool tSuccess = _TestProcess(pMessage,
                               (!pFail && pFound == pExp) ||
                               (pFail && pFound != pExp));
  
  if (tSuccess)
  {
    if (!pFail)
      fprintf(stdout, ": %u", pFound);
    else
      fprintf(stdout, ": expected not %u, found %u", \
              pExp, pFound);
  }
  else
  {
    if (!pFail)
      fprintf(stdout, ": expected %u, found %u", pExp, pFound);
    else
      fprintf(stdout, ": expected not %u", pExp);
  }
  fputs("\n", stdout);
  
  return tSuccess;
}

bool TestCompareUInts(uint32_t pFound, uint32_t pExp, const char* pMessage)
{
  return _TestCompareUInts(pFound, pExp, pMessage, false);
}

bool TestCompareUIntsNot(uint32_t pFound, uint32_t pExp, const char* pMessage)
{
  return _TestCompareUInts(pFound, pExp, pMessage, true);
}

const char* kTrue = "true";
const char* kFalse = "false";

const char* _boolAsString(bool pBool)
{
  if (pBool)
    return kTrue;
  else
    return kFalse;
}

bool _TestCompareBools(bool pFound, bool pExpected, const char *pMessage, bool pFail)
{
  bool tSuccess = _TestProcess(pMessage,
                               (!pFail && pFound == pExpected) ||
                               (pFail && pFound != pExpected));
  
  if (tSuccess)
  {
    if (!pFail)
      fprintf(stdout, ": %s", _boolAsString(pFound));
    else
      fprintf(stdout, ": expected not %s, found %s", \
              _boolAsString(pExpected), _boolAsString(pFound));
  }
  else
  {
    if (!pFail)
      fprintf(stdout, ": expected %s, found %s", _boolAsString(pExpected),
              _boolAsString(pFound));
    else
      fprintf(stdout, ": expected not %s", _boolAsString(pExpected));
  }
  fputs("\n", stdout);
  return tSuccess;
}

bool TestCompareBools(bool pFound, bool pExpected, const char* pMessage)
{
  return _TestCompareBools(pFound, pExpected, pMessage, false);
}
bool TestCompareBoolsNot(bool pFound, bool pExpected, const char* pMessage)
{
  return _TestCompareBools(pFound, pExpected, pMessage, true);
}
bool check_error(int pErrorCode, const char *pError)
{
  if (pErrorCode > MAILIMAP_NO_ERROR_NON_AUTHENTICATED)
  {
    TestAssert(pError, false);
    return false;
  }
  return true;
}

void _print(const char *pMsg)
{
  fprintf(stdout, "%s\n", pMsg);
}


////////////////////////////////////////////////////////////////////////////////
//
// Test functions
//
////////////////////////////////////////////////////////////////////////////////


/*
 * LIST command
 */
void test_mailbox_list(mailimap* pSession, const char *pMailbox, uint32_t pExpectedCount)
{
  fprintf(stdout, "Listing mailbox '%s'\n", pMailbox);
  
  clist *tList;
  int r;
  
  size_t tMailboxLength;
  tMailboxLength = strlen(pMailbox);
  
  char *tMailboxPath;
  if (tMailboxLength != 0)
  {
    tMailboxPath = (char*) malloc(tMailboxLength + 2);
  
    if (tMailboxPath != NULL)
    {
      strcpy(tMailboxPath, pMailbox);
      tMailboxPath[tMailboxLength] = '.';
      tMailboxPath[tMailboxLength + 1] = '\0';
    }
  }
  else
    tMailboxPath = strdup(pMailbox);
  
  if (tMailboxPath == NULL)
  {
    fprintf(stdout, "Cannot allocate memory");
    return;
  }
  
  r = mailimap_list(pSession, tMailboxPath, "%", &tList);
  check_error(r, "could not list mailboxes in INBOX");
  
  uint32_t tCount = 0;
  if (tList != NULL)
  {
    clistiter *cur;
    
    for (cur = clist_begin(tList); cur != NULL; cur = clist_next(cur))
    {
      char *tFlags, *tName, *tDel;
      struct mailimap_mailbox_list *tMbxLst;
      
      tCount++;
      tMbxLst = (struct mailimap_mailbox_list*) clist_content(cur);
      r = mailimap_fetch_mailbox_details(tMbxLst, &tName, &tFlags, &tDel);
      check_error(r, "could not fetch details");
    
      TestAssert("Name returned", tName != NULL);
      TestAssert("Flags returned", tFlags != NULL);
      TestAssert("Delimiter returned", tDel != NULL);
    }
    
    mailimap_list_result_free(tList);
  }
  
  TestCompareUInts(tCount, pExpectedCount, "Maiboxes found");
  free(tMailboxPath);
}


/*
 * STATUS command
 */
void test_mailbox_status(struct mailimap *pSession, const char* pMailbox, bool pFail)
{
  int tCode;
  struct mailimap_status_bridge tStatus;
  tCode = mailimap_fetch_mailbox_status(pSession, pMailbox, &tStatus);
  
  if (tCode != MAILIMAP_NO_ERROR)
  {
    TestAssert("call to mailimap_fetch_mailbox_status failed as expected " \
              "with incorrect mailbox name", \
               pFail);
  }
  else
  {
    if (!pFail)
    {
      TestCompareUIntsNot(tStatus.sb_messages, 0, "Messages");
      TestCompareUIntsNot(tStatus.sb_uidnext, 0, "UIDNext");
      TestCompareUIntsNot(tStatus.sb_uidvalidity, 0, "UIDValidity");
    }
    else
    {
      TestCompareUIntsNot(tStatus.sb_messages, 0, "Messages");
      TestCompareUIntsNot(tStatus.sb_uidnext, 0, "UIDNext");
      TestCompareUIntsNot(tStatus.sb_uidvalidity, 0, "UIDValidity");
    }
  }
}

/*
 * SELECT command
 */
void test_mailbox_select(struct mailimap *pSession, const char *pMailbox)
{
  int tCode;
  struct mailimap_mailbox_info_bridge tInfo;
  char* tFlags;
  tCode = mailimap_fetch_mailbox_info(pSession, pMailbox, &tInfo, &tFlags);
  
  if (check_error(tCode, "mailimap_fetch_mailbox_info"))
  {
    TestCompareIntsNot(tInfo.sel_perm, 1, "perm");
    TestCompareUIntsNot(tInfo.sel_exists, 1, "exists");
    TestCompareUIntsNot(tInfo.sel_unseen, 1, "unseen");
    TestCompareUIntsNot(tInfo.sel_uidnext, 1, "uidnext");
    TestCompareUIntsNot(tInfo.sel_uidvalidity, 1, "uidvalidity");
  }
}


/*
 * SEARCH command
 */
void __runSearch(struct mailimap *pSession,
                 int (*search)(struct mailimap *pSession, struct mailimap_search_key* pKey,
                               char**rResults),
                 const char *pFuncName)
{
  int r;
  
  r = mailimap_select(pSession, "INBOX");
  
  if (!check_error(r, "mailimap_select INBOX"))
    return;
  
  struct mailimap_set *tSet = mailimap_set_new_interval(1, 7);
  
  if (!TestAssert("mailimap_set_new_interval succeeded", tSet != NULL))
    return;
  
  struct mailimap_search_key *tKey;
  tKey = mailimap_search_key_new_set(tSet);
  
  if (TestAssert("mailimap_search_key_new_set succeeded", tKey != NULL))
  {
    char *tResults = NULL;
    r = search(pSession, tKey, &tResults);
    
    if (check_error(r, pFuncName))
    {
      if (TestAssert("results are not empty", tResults != NULL))
      {
        _print(tResults);
        TestCompareUIntsNot((uint32_t)strlen(tResults), 0, "results returned");
        free(tResults);
      }
    }
  }
  
  mailimap_search_key_free(tKey);
}

void test_search(struct mailimap *pSession)
{
  __runSearch(pSession, &mailimap_run_search, "mailimap_run_search");
}

void test_uid_search(struct mailimap *pSession)
{
  __runSearch(pSession, &mailimap_run_uid_search, "mailimap_run_uid_search");
}


/*
 * FETCH command
 */
uint32_t sCount;
uint32_t sContext = 42;

void __fetch_reset_count()
{
  sCount = 0;
}

void __fetch_add_item()
{
  sCount++;
}

uint32_t __fetch_get_count()
{
  return sCount;
}

void _fetch_email_callback(uint32_t pUID, uint32_t pSize, char *pSubject,
                           char *pInternalDate, char *pError, uint32_t pContext)
{
  TestCompareUIntsNot(pUID, 0, "UID returned");
  TestCompareUIntsNot(pSize, 0, "Size returned");
  
  TestAssert("Subject is not null", pSubject != NULL);
  if (pSubject != NULL)
    TestCompareUIntsNot((uint32_t)strlen(pSubject), 0, "Subject not empty");
  
  TestAssert("pInternalDate is not null", pInternalDate != NULL);
  if (pInternalDate != NULL)
    TestCompareUIntsNot((uint32_t)strlen(pInternalDate), 0,
                        "Internal date not empty");
  
  TestAssert("No error returned", pError == NULL);
  
  if (pError != NULL)
    fprintf(stdout, "Error returned: %s\n", pError);
  
  TestCompareUInts(pContext, sContext, "Context");
  __fetch_add_item();
}
          

void test_fetch_email(struct mailimap *pSession)
{
  struct mailimap_set* tSet = mailimap_set_new_single(6);
  
  if (tSet == NULL)
  {
    _print("Error!");
    return;
  }
  
  int r;
  r = mailimap_select(pSession, "INBOX");
  
  TestAssert("SELECT succeeded", r == MAILIMAP_NO_ERROR);
  
  if (r != MAILIMAP_NO_ERROR)
    return;
  
  __fetch_reset_count();
  
  r = mailimap_fetch_email_details(pSession, tSet, &_fetch_email_callback, sContext);
  TestAssert("mailimap_fetch_email_details succeeded", r == MAILIMAP_NO_ERROR);
  TestCompareUInts(__fetch_get_count(), 1, "single UID fetch");
  mailimap_set_free(tSet);
  
  // Use interval set
  tSet = mailimap_set_new_interval(1, 7);
  
  if (tSet == NULL)
  {
    _print("Error!");
    return;
  }
  
  __fetch_reset_count();
  r = mailimap_fetch_email_details(pSession, tSet, &_fetch_email_callback, sContext);
  TestAssert("mailimap_fetch_email_details succeeded", r == MAILIMAP_NO_ERROR);
  TestCompareUInts(__fetch_get_count(), 3, "interval fetch");
  mailimap_set_free(tSet);
  
  
  // Use a wrong UID - message UID 1 no longer exists
  tSet = mailimap_set_new_single(1);
  
  if (tSet == NULL)
  {
    _print("Error!");
    return;
  }
  
  uint32_t tID = 42;
  __fetch_reset_count();
  r = mailimap_fetch_email_details(pSession, tSet, &_fetch_email_callback, tID);
  TestAssert("mailimap_fetch_email_details succeeded", r == MAILIMAP_NO_ERROR);
  TestCompareUInts(__fetch_get_count(), 0, "non-existent UID fetch");
  mailimap_set_free(tSet);
}



/*
 * Main
 */
int main(int argc, char **argv)
{
  struct mailimap * imap;
  int r;
  
  TestInitialise();
  
  _print("Start!");
  imap = mailimap_new(0, NULL);
  r = mailimap_ssl_connect(imap, "molly.livecode.com", 993);
  fprintf(stderr, "connect: %i\n", r);
  check_error(r, "could not connect to server");
  
  r = mailimap_login(imap, "quality@kognition.io", "Qr3}b9l3,s=t");
  check_error(r, "could not login");
  
  test_fetch_email(imap);
  
  test_mailbox_list(imap, "", 1);
  test_mailbox_list(imap, "INBOX", 9);

  test_mailbox_status(imap, "INBOX.Google", false);
  test_mailbox_status(imap, "invalidaleinfsel", true);

  test_mailbox_select(imap, "INBOX");
  test_mailbox_select(imap, "");
  
  test_search(imap);
  test_uid_search(imap);
  
  TestPrintResults();
  
  mailimap_logout(imap);
  mailimap_free(imap);
  
  exit(EXIT_SUCCESS);
}

