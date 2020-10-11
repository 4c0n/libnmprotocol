/*
    Nord Modular Midi Protocol 3.03 Library
    Copyright (C) 2003 Marcus Andersson

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef ACTIVEPIDLISTENER_H
#define ACTIVEPIDLISTENER_H

#include "nmprotocol/nmprotocollistener.h"

#include <map>

class ActivePidListener : public NMProtocolListener
{
 public:

  ActivePidListener();
  virtual ~ActivePidListener();

  virtual void messageReceived(AckMessage message);
  virtual void messageReceived(NewPatchInSlotMessage message);

  int getActivePid(int slot);

 private:

  typedef map<int, int> ActivePidMap;

  ActivePidMap activePidMap;
};

#endif
