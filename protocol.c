
#include "arachne.h"
#include "internet.h"

int protocol_arachne(struct HTTPrecord *cacheitem,struct Url *url,int *returnvalue)
{
 char *value;
#ifndef NOTCPIP
#ifndef POSIX
  if(!strncmpi(url->file,"ftp-",4))
  {
   char back=1;

   if(!strncmpi(&(url->file[4]),"send",4))
    back=0;

   strcpy(url->user, config_get_str("FTPusername", ""));
   strcpy(url->password, config_get_str("FTPpassword", ""));
   strcpy(url->host, config_get_str("FTPserver", ""));
   strcpy(url->file, config_get_str("FTPpath", ""));
   url->port=21;

   if(tcpip)
    ftpsession(url,cacheitem,LASTlocname);

   if(!GLOBAL.gotolocation)
   {
    if(back)
     goback();
    else
     strcpy(GLOBAL.location,p->htmlframe[p->activeframe].cacheitem.URL);
    arachne.target=p->activeframe;
   }

   GLOBAL.reload=0;
   GLOBAL.postdata=0;
   return GOTO_IVEGOTNEWURL;
  }
#endif
#endif
#ifndef POSIX
  if(!strcmpi(url->file,"restart"))
  {
   char buf[IE_MAXLEN];
   char *ptr=getenv("ASETUP");

   if(ptr && !strcmp(ptr,"inst"))  //special case - "Finish Setup" button
    return GOTO_USEREND;

   arachne.target=0; //!!!
   strcpy(buf,"@arachne");
   if(tcpip)
    strcat(buf," -o\n");
   else
    strcat(buf,"\n");
   unlink("lock");
   *returnvalue=willexecute(buf);
   return GOTO_END;
  }
  else
#endif
 ///!!!!
 ///POSIX restart: TerminateArachne + exec.. (argv[0],argv[0],NULL) !
#ifndef NOTCPIP
  if(!strcmpi(url->file,"dialpage"))
  {
   value = config_get_str("DialPage", "file:ppp_init.htm");

//!!glennmcc: Begin Feb 06, 2005
//also use ppp_init.htm if DialPage does not begin with file:ppp
//indicating that it has been changed from one of the 4 included dialpages
//which are... ppp_init.htm, pppenhan.htm, pppframe.htm or ppp_fast.htm
//value=strlwr(value);
if(!strstr(value,"file:ppp")) value="file:ppp_init.htm";
//!!glennmcc: end

   if(value)
   {
    strcpy(GLOBAL.location,value);
    AnalyseURL(GLOBAL.location,url,IGNORE_PARENT_FRAME);
   }
  }
  else
#ifndef POSIX
  if(!strcmpi(url->file,"dialer"))
  {
   *returnvalue=willexecute(ArachneDIAL());
   return GOTO_END;
  }
  else
  if(!strcmpi(url->file,"hangup"))
  {
   char buf[IE_MAXLEN];
   arachne.target=0; //!!!
   outs(MSG_HANGUP);
   if(tcpip)
    PPPtimelog();
   process_form(0,IE_NULL); //updateovat Arachne.Cfg
   sprintf(buf,"%s\nif exist PPP.LOG del PPP.LOG\n",
    config_get_str("Hangup", ""));
   if(!config_get_bool("ExitOnHangup", 0))
    strcat(buf,"@arachne -c\n");
   else
    exitmsg();
   *returnvalue=willexecute(buf);
   return GOTO_END;
  }
  else
#endif
#endif //NOTCPIP
  if(!strncmpi(url->file,"exit",4))
  {
   if(url->file[4]=='=' || url->file[4]=='?')
    *returnvalue=atoi(&(url->file[5]));
   return GOTO_USEREND;
  }
 return 0;
}



int protocol_nohttp(struct HTTPrecord *cacheitem,struct Url *url, unsigned *cacheitem_status, XSWAP *cacheitem_writeadr)
{
 char *value;

#ifndef POSIX
#ifdef OVRL
#ifndef XTVERSION
 //---------------------------------------------------------------- finger:
 if(!strcmpi(url->protocol,"finger"))
 //------------------------------------------------------------------------
 {
  //finger to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);
  strcpy(cacheitem->mime,"finger/out");
  if(xfinger(url,cacheitem,url->user))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 else if(!strcmpi(url->protocol,"news") ||
         !strcmpi(url->protocol,"nntp") )
 {
  //finger to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);

  if(!url->host[0])
  {
   value = config_get_str("NNTPserver", NULL);
   if(value)
   {
    makestr(url->host,value,STRINGSIZE-1);
   }
  }

  if(!url->file[0] || url->file[0]=='/' && !url->file[1])
   strcpy(url->file,"LIST");

  strcpy(cacheitem->mime,"news/list");
  if(xfinger(url,cacheitem,url->file))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 //---------------------------------------------------------------- gopher:
 else if(!strcmpi(url->protocol,"gopher"))
 //------------------------------------------------------------------------
 {
  char *selector;
  char dummy[IE_MAXLEN+1];

  int plugin=externalprotocol(url->protocol,dummy);
  if(plugin)
   return GOTO_EXTERNAL;

  //finger to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);

  selector=&(url->file[1]);
  if(!url->file[1])
  {
   strcpy(cacheitem->mime,"gopher/1");
  }
  else if(url->file[2]=='/')
  {
   sprintf(cacheitem->mime,"gopher/%c",url->file[1]);
   if(url->file[1]=='7' && !strchr(url->file,'?')) //search
    return GOTO_TRYPLUGIN;
   selector=&(url->file[3]);
  }

  if(xfinger(url,cacheitem,selector))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 else
#endif //XTVERSION
#endif //statical version
 //------------------------------------------------------------------- ftp:
 if(!strcmpi(url->protocol,"ftp"))
 //------------------------------------------------------------------------
 {
  char *ptr;
  char dummy[IE_MAXLEN+1];
  int plugin=externalprotocol(url->protocol,dummy);
  if(plugin)
   return GOTO_EXTERNAL;

  if(url->user[0] && !url->password[0] &&
     (strcmp(url->host,AUTHENTICATION->host) ||
      strcmp(url->user,AUTHENTICATION->user) ||
      strcmp(AUTHENTICATION->realm,"$ftp")))
  {
   sprintf(cacheitem->locname,"%s%sftplogin.ah",sharepath,GUIPATH);
   strcpy(AUTHENTICATION->host,url->host);
   return GOTO_LOCAL_HTML;
  }

  if (config_get_bool("UseFTPproxy", 0))
  {
   char *no4all = config_get_str("NoFTPproxy4all", NULL);
   ptr = config_get_str("NoFTPproxy", NULL);
   if((!ptr || !strstr(strlwr(ptr),strlwr(url->host) )) &&
      (!no4all || !strstr(strlwr(url->host), strlwr(no4all)) ) )
    return GOTO_PROXY;
  }

  //ftp to cache
  *cacheitem_writeadr=Write2Cache(url,cacheitem,1,1);

  if(ftpsession(url,cacheitem,NULL))
  {
   if(GLOBAL.abort)
    return GOTO_ABORT;
   UpdateInCache(*cacheitem_writeadr,cacheitem);
   *cacheitem_status=REMOTE;
  }
  else
  {
   GLOBAL.reload=0;
   return GOTO_ABORT;
  }
 }
 //------------------------------------------------------------------ pop3:
 else if(!strcmpi(url->protocol,"pop3"))
 //------------------------------------------------------------------------
 {
  char dele=1,log=0;

  if(!url->host[0])
  {
   value = config_get_str("POP3server", NULL);
   if(value)
    makestr(url->host,value,STRINGSIZE-1);
  }
  if(!url->user[0])
  {
   value = config_get_str("POP3username", NULL);
   if(value)
    makestr(url->user,value,STRINGSIZE-1);
  }

  //Draw title and URL (without password):

  MakeTitle(MSG_MAILDL);
  url2str(url,GLOBAL.location);
  SetInputAtom(&URLprompt,GLOBAL.location);
  DrawTitle(0);
  add2history(GLOBAL.location);

  //Get password and other settings:

  if(!url->password[0])
  {
   value = config_get_str("POP3password", NULL);
   if(value)
    makestr(url->password,value,PASSWORDSIZE-1);
  }

  if (config_get_bool("KeepOnServer", 0))
   dele = 0;

  if (config_get_bool("POP3log", 0))
   log = 1;

  //POP3 download is performed here:

  if(!xpopdump(url,dele,log))
  {
   sprintf(p->htmlframe[0].cacheitem.locname,"%s%serr_pop3.ah",sharepath,GUIPATH);
   return GOTO_ERROR;
  }
  else if(arachne.scriptline==0)
  {
   strcpy(GLOBAL.location, config_get_str("AfterPOP3", "file://inbox.dgi"));
   GLOBAL.reload=RELOAD_CURRENT_LOCATION;
   return GOTO_IVEGOTNEWURL;
  }
  else
   return GOTO_READSCRIPT;
 }
 //------------------------------------------------------------------ smtp:
 else if(!strcmpi(url->protocol,"smtp"))
 //------------------------------------------------------------------------
 {
  char helo=1,log=0;

  if(!url->host[0])
  {
   value = config_get_str("SMTPserver", NULL);
   if(value)
    makestr(url->host,value,STRINGSIZE-1);
  }

//!!glennmcc: Feb 13, 2006 -- 'SendHelo' is more logical ;-)
  helo = config_get_bool("SendHELO", 1);
//  value=configvariable(&ARACHNEcfg,"NoHELO",NULL);
//  if(value && toupper(*value)=='Y') helo=0;
//!!glennmcc: end

  if (config_get_bool("SMTPlog", 0))
   log = 1;

//!!glennmcc: begin Nov 09, 2003 --- for Authenticated SMTP
//!!glennmcc: Feb 17, 2006 -- moved down below
/*
  value=configvariable(&ARACHNEcfg,"UseAuthSMTP",NULL);
  if(!value || toupper(*value)!='N') helo=2;
*/
//!!glennmcc: end

  // for authenticated SMTP
  value = config_get_str("AuthSMTPusername", "");
  makestr(url->authuser,value,STRINGSIZE-1);
  value = config_get_str("AuthSMTPpassword", "");
  makestr(url->authpassword,value,PASSWORDSIZE-1);
  if(config_get_bool("UseAuthSMTP", 0) && strlen(url->authuser) > 0 && strlen(url->authpassword) > 0)
     helo=2;
  else
     helo=1;

  if(!url->user[0])
  {
   value = config_get_str("eMail", "");
   makestr(url->user,value,STRINGSIZE-1);
  }

  if(!url->file[0] || !url->file[1] ) //stmp: or smtp:/
  {
   value = config_get_str("MailPath", "MAIL\\");
   if(value)
   {
    sprintf(url->file,"/%s*.TBS",value);
   }
  }

  //Draw title and URL:

  MakeTitle(MSG_MAILUP);
  url2str(url,GLOBAL.location);
  SetInputAtom(&URLprompt,GLOBAL.location);
  DrawTitle(0);
  add2history(GLOBAL.location);

  //SMTP upload is performed here:

  if(!xsendmail(url,helo,log))

  {
   sprintf(p->htmlframe[0].cacheitem.locname,"%s%serr_smtp.ah",sharepath,GUIPATH);
   return GOTO_ERROR;
  }
  else if(arachne.scriptline==0)
  {
   if(strchr(url->file,'*') || GLOBAL.mailaction & MAIL_OUTBOXNOW)
   {
    strcpy(GLOBAL.location, config_get_str("AfterSMTP", "file://outbox.dgi"));
    GLOBAL.reload=RELOAD_CURRENT_LOCATION;
   }
   else
   {
//!!glennmcc: Feb 27, 2005
// since 'smtp:' is not being saved into history, 2 goback()s when we hit
// 'send mail now' on any of the mail compose screens ends-up taking us
// back one step too far.
// Therefore, we only goback once when it's not being sent 'on the spot'
    if(!strstr(GLOBAL.location,"smtp:")) goback(); //return to mailto: page...
//!!glennmcc: end

    goback(); //return to page with <A HREF=mailto:...> tag...
    GLOBAL.postdata=0;

// RAY: This is done automaticaly now for inbox and outbox,
// see guivent.c
//    if(!strcmp(GLOBAL.location,"file://outbox.dgi"))
//     GLOBAL.reload=RELOAD_CURRENT_LOCATION;
//    else
//Ray: end
     GLOBAL.reload=NO_RELOAD;

   }
   return GOTO_IVEGOTNEWURL;
  }
  else
   return GOTO_READSCRIPT;
 }
 else
#endif
 //--------------------------------------------------------------- error?
  return UNKNOWN_PROTOCOL;
#ifndef POSIX
 return CONTINUE_TO_RENDER;
#endif
}
