/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd is copyright (C) 2002-2006 ChatSpike-Dev.
 *                       E-mail:
 *                <brain@chatspike.net>
 *           	  <Craig@chatspike.net>
 *     
 * Written by Craig Edwards, Craig McLure, and others.
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

using namespace std;

#include <stdio.h>
#include "users.h"
#include "channels.h"
#include "modules.h"
#include "helperfuncs.h"
#include "dns.h"

/* $ModDesc: Povides a proof-of-concept test /WOOT command */

class MyResolver : public Resolver
{
 public:
	MyResolver(const std::string &source, bool forward) : Resolver(source, forward) { }

	virtual void OnLookupComplete(const std::string &result)
	{
		log(DEBUG,"*** RESOLVER COMPLETED LOOKUP, IP IS: '%s'",result.c_str());
	}

	virtual void OnError(ResolverError e)
	{
		log(DEBUG,"*** RESOLVER GOT ERROR: %d",e);
	}
};

static Server *Srv;
	 
class cmd_woot : public command_t
{
 public:
	cmd_woot () : command_t("WOOT", 0, 0)
	{
		this->source = "m_testcommand.so";
	}

	void Handle (const char** parameters, int pcnt, userrec *user)
	{
		/* We dont have to worry about deleting 'r', the core will
		 * do it for us as required.*/

		try
		{
			MyResolver* r = new MyResolver("brainbox.ath.cx", true);
			Srv->AddResolver(r);
		}
		catch (ModuleException& e)
		{
			log(DEBUG,"Danger, will robinson! There was an exception: %s",e.GetReason());
		}
	}
};

class ModuleTestCommand : public Module
{
	cmd_woot* newcommand;
 public:
	ModuleTestCommand(Server* Me)
		: Module::Module(Me)
	{
		Srv = Me;
		// Create a new command:
		// command will be called /WOOT, and will
		// call handle_woot when triggered, the
		// 0 in the modes parameter signifies that
		// anyone can issue the command, and the
		// command takes only one parameter.
		newcommand = new cmd_woot();
		Srv->AddCommand(newcommand);
	}

	void Implements(char* List)
	{
		List[I_OnUserJoin] = 1;
	}

	virtual void OnUserJoin(userrec* user, chanrec* channel)
	{
	}
	
	virtual ~ModuleTestCommand()
	{
	}
	
	virtual Version GetVersion()
	{
		return Version(1,0,0,0,VF_STATIC|VF_VENDOR);
	}
};


class ModuleTestCommandFactory : public ModuleFactory
{
 public:
	ModuleTestCommandFactory()
	{
	}
	
	~ModuleTestCommandFactory()
	{
	}
	
	virtual Module * CreateModule(Server* Me)
	{
		return new ModuleTestCommand(Me);
	}
	
};


extern "C" void * init_module( void )
{
	return new ModuleTestCommandFactory;
}

