//
//
//

#include <stdio.h>
#include "mailimap_wrap.h"

#include <libetpan/libetpan.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

void TestPrintResults(void);
void TestAssert(const char* pMessage, bool pTest);

void TestCompareInts(int pFound, int pExpected, const char* pMessage);
void TestCompareIntsNot(int pFound, int pExpected, const char* pMessage);

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
  
  fprintf(stdout, "%sOK - %s: ", tSuccess, pMessage);
  return t;
}

void TestAssert(const char* pMessage, bool pTest)
{
  _TestProcess(pMessage, pTest);
  fputs("\n", stdout);
}

void _TestCompareInts(int pFound, int pExpected, const char* pMessage, bool pFail)
{
  bool tSuccess = _TestProcess(pMessage,
                              (!pFail && pFound == pExpected) ||
                               (pFail && pFound != pExpected));

  if (tSuccess)
  {
    if (!pFail)
      fprintf(stdout, "%i", pFound);
    else
      fprintf(stdout, "expected not %i, found %i", \
            pExpected, pFound);
  }
  else
  {
    if (!pFail)
      fprintf(stdout, "expected %i, found %i", pExpected, pFound);
    else
      fprintf(stdout, "expected not %i", pExpected);
  }
  fputs("\n", stdout);
}

void TestCompareInts(int pFound, int pExpected, const char* pMessage)
{
  _TestCompareInts(pFound, pExpected, pMessage, false);
}

void TestCompareIntsNot(int pFound, int pExpected, const char* pMessage)
{
  _TestCompareInts(pFound, pExpected, pMessage, true);
}

void _TestCompareUInts(uint32_t pFound, uint32_t pExp, const char* pMessage, bool pFail)
{
  bool tSuccess = _TestProcess(pMessage,
                               (!pFail && pFound == pExp) ||
                               (pFail && pFound != pExp));
  
  if (tSuccess)
  {
    if (!pFail)
      fprintf(stdout, "%u", pFound);
    else
      fprintf(stdout, "expected not %u, found %u", \
              pExp, pFound);
  }
  else
  {
    if (!pFail)
      fprintf(stdout, "expected %u, found %u", pExp, pFound);
    else
      fprintf(stdout, "expected not %u", pExp);
  }
  fputs("\n", stdout);
}

void TestCompareUInts(uint32_t pFound, uint32_t pExp, const char* pMessage)
{
  _TestCompareUInts(pFound, pExp, pMessage, false);
}

void TestCompareUIntsNot(uint32_t pFound, uint32_t pExp, const char* pMessage)
{
  _TestCompareUInts(pFound, pExp, pMessage, true);
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

void _TestCompareBools(bool pFound, bool pExpected, const char *pMessage, bool pFail)
{
  bool tSuccess = _TestProcess(pMessage,
                               (!pFail && pFound == pExpected) ||
                               (pFail && pFound != pExpected));
  
  if (tSuccess)
  {
    if (!pFail)
      fprintf(stdout, "%s", _boolAsString(pFound));
    else
      fprintf(stdout, "expected not %s, found %s", \
              _boolAsString(pExpected), _boolAsString(pFound));
  }
  else
  {
    if (!pFail)
      fprintf(stdout, "expected %s, found %s", _boolAsString(pExpected),
              _boolAsString(pFound));
    else
      fprintf(stdout, "expected not %s", _boolAsString(pExpected));
  }
  fputs("\n", stdout);
}

void TestCompareBools(bool pFound, bool pExpected, const char* pMessage)
{
  _TestCompareBools(pFound, pExpected, pMessage, false);
}
void TestCompareBoolsNot(bool pFound, bool pExpected, const char* pMessage)
{
  _TestCompareBools(pFound, pExpected, pMessage, true);
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
  
  test_mailbox_list(imap, "", 1);
  test_mailbox_list(imap, "INBOX", 9);
  
  test_mailbox_status(imap, "INBOX.Google", false);
  test_mailbox_status(imap, "invalidaleinfsel", true);
  
  test_mailbox_select(imap, "INBOX");
  test_mailbox_select(imap, "");
  
  TestPrintResults();
  
  mailimap_logout(imap);
  mailimap_free(imap);
  
  exit(EXIT_SUCCESS);
}

