/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2007 Robin Burchell <robin+git@viroteck.net>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "inspircd.h"

/** Handle /LIST. These command handlers can be reloaded by the core,
 * and handle basic RFC1459 commands. Commands within modules work
 * the same way, however, they can be fully unloaded, where these
 * may not.
 */
class CommandList : public Command
{
	ChanModeReference secretmode;
	ChanModeReference privatemode;

 public:
	/** Constructor for list.
	 */
	CommandList(Module* parent)
		: Command(parent,"LIST", 0, 0)
		, secretmode(creator, "secret")
		, privatemode(creator, "private")
	{
		Penalty = 5;
	}

	/** Handle command.
	 * @param parameters The parameters to the comamnd
	 * @param pcnt The number of parameters passed to teh command
	 * @param user The user issuing the command
	 * @return A value from CmdResult to indicate command success or failure.
	 */
	CmdResult Handle(const std::vector<std::string>& parameters, User *user);
};


/** Handle /LIST
 */
CmdResult CommandList::Handle (const std::vector<std::string>& parameters, User *user)
{
	int minusers = 0, maxusers = 0;

	user->WriteNumeric(RPL_LISTSTART, "Channel :Users Name");

	/* Work around mIRC suckyness. YOU SUCK, KHALED! */
	if (parameters.size() == 1)
	{
		if (parameters[0][0] == '<')
		{
			maxusers = atoi((parameters[0].c_str())+1);
		}
		else if (parameters[0][0] == '>')
		{
			minusers = atoi((parameters[0].c_str())+1);
		}
	}

	for (chan_hash::const_iterator i = ServerInstance->chanlist->begin(); i != ServerInstance->chanlist->end(); i++)
	{
		// attempt to match a glob pattern
		long users = i->second->GetUserCounter();

		bool too_few = (minusers && (users <= minusers));
		bool too_many = (maxusers && (users >= maxusers));

		if (too_many || too_few)
			continue;

		if (parameters.size() && !parameters[0].empty() && (parameters[0][0] != '<' && parameters[0][0] != '>'))
		{
			if (!InspIRCd::Match(i->second->name, parameters[0]) && !InspIRCd::Match(i->second->topic, parameters[0]))
				continue;
		}

		// if the channel is not private/secret, OR the user is on the channel anyway
		bool n = (i->second->HasUser(user) || user->HasPrivPermission("channels/auspex"));

		if (!n && i->second->IsModeSet(privatemode))
		{
			/* Channel is +p and user is outside/not privileged */
			user->WriteNumeric(RPL_LIST, "* %ld :", users);
		}
		else
		{
			if (n || !i->second->IsModeSet(secretmode))
			{
				/* User is in the channel/privileged, channel is not +s */
				user->WriteNumeric(RPL_LIST, "%s %ld :[+%s] %s",i->second->name.c_str(),users,i->second->ChanModes(n),i->second->topic.c_str());
			}
		}
	}
	user->WriteNumeric(RPL_LISTEND, ":End of channel list.");

	return CMD_SUCCESS;
}


COMMAND_INIT(CommandList)
