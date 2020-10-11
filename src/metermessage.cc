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

#include "nmprotocol/metermessage.h"
#include "nmprotocol/nmprotocollistener.h"
#include "nmprotocol/midiexception.h"
#include "pdl/packet.h"

MeterMessage::MeterMessage()
{
  cc = 0x14;
  slot = 0;
  pid = 0;
  startIndex = 0;
}

MeterMessage::MeterMessage(Packet* packet)
{
  cc = packet->getVariable("cc");
  slot = packet->getVariable("slot");
  pid = packet->getVariable("data:pid");
  startIndex = packet->getVariable("data:data:startIndex");
  meters[0] = packet->getVariable("data:data:b0");
  meters[1] = packet->getVariable("data:data:a0");
  meters[2] = packet->getVariable("data:data:b1");
  meters[3] = packet->getVariable("data:data:a1");
  meters[4] = packet->getVariable("data:data:b2");
  meters[5] = packet->getVariable("data:data:a2");
  meters[6] = packet->getVariable("data:data:b3");
  meters[7] = packet->getVariable("data:data:a3");
}

MeterMessage::~MeterMessage()
{
}

void MeterMessage::getBitStream(BitStreamList* bitStreamList)
{
  throw MidiException("MeterMessage::getBitStream not implemented.", 0);
}

void MeterMessage::notifyListener(NMProtocolListener* listener)
{
  listener->messageReceived(*this);
}

int MeterMessage::getStartIndex()
{
  return startIndex;
}

int MeterMessage::getMeterStatus(int meterNo)
{
  if (meterNo >= 0 && meterNo <= 19) {
    return meters[meterNo];
  }
  return 0;
}

int MeterMessage::getPid()
{
  return pid;
}
