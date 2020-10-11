/*
    Nord Modular Midi Protocol 3.03 Library
    Copyright (C) 2003-2011 Marcus Andersson

    with parts from

    amidi.c - read from/write to RawMIDI ports
    Copyright (c) Clemens Ladisch <clemens@ladisch.de>

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

#ifdef ALSA

#include <unistd.h>
#include <glob.h>
#include <poll.h>

#include "nmprotocol/midiexception.h"
#include "nmprotocol/alsadriver.h"

ALSADriver::ALSADriver()
{
}

ALSADriver::~ALSADriver()
{
  disconnect();
}

static void error(const char *format, ...)
{
  va_list ap;
  
  va_start(ap, format);
  vfprintf(stderr, format, ap);
  va_end(ap);
  putc('\n', stderr);
}

ALSADriver::StringList ALSADriver::list_device(snd_ctl_t *ctl, int card,
				   int device, snd_rawmidi_stream_t mode)
{
  snd_rawmidi_info_t *info;
  const char *name;
  const char *sub_name;
  int subs;
  int err;
  StringList ports;
  char buf[40];
  
  snd_rawmidi_info_alloca(&info);
  snd_rawmidi_info_set_device(info, device);
  snd_rawmidi_info_set_subdevice(info, 0);
  snd_rawmidi_info_set_stream(info, mode);
  if ((err = snd_ctl_rawmidi_info(ctl, info)) < 0 &&
      err != -ENOENT) {
    error("cannot get rawmidi information %d:%d: %s",
	  card, device, snd_strerror(err));
    return ports;
  }
  if (err == -ENOENT)
    return ports;
  name = snd_rawmidi_info_get_name(info);
  sub_name = snd_rawmidi_info_get_subdevice_name(info);
  subs = snd_rawmidi_info_get_subdevices_count(info);
  if (sub_name[0] == '\0') {
    snprintf(buf, 40, "hw:%d,%d", card, device);
    ports.push_back(buf);
  } else {
    int sub = 0;
    for (;;) {
      snprintf(buf, 40, "hw:%d,%d,%d", card, device, sub);
      ports.push_back(buf);
      if (++sub >= subs)
	break;
      snd_rawmidi_info_set_subdevice(info, sub);
      if ((err = snd_ctl_rawmidi_info(ctl, info)) < 0) {
	error("cannot get rawmidi information %d:%d:%d: %s",
	      card, device, sub, snd_strerror(err));
	break;
      }
      sub_name = snd_rawmidi_info_get_subdevice_name(info);
    }
  }

  return ports;
}

ALSADriver::StringList ALSADriver::list_card_devices(int card,
						     snd_rawmidi_stream_t mode)
{
  snd_ctl_t *ctl;
  char name[32];
  int device;
  int err;
  StringList ports;
  
  sprintf(name, "hw:%d", card);
  if ((err = snd_ctl_open(&ctl, name, 0)) < 0) {
    error("cannot open control for card %d: %s", card, snd_strerror(err));
    return ports;
  }
  device = -1;
  for (;;) {
    if ((err = snd_ctl_rawmidi_next_device(ctl, &device)) < 0) {
      error("cannot determine device number: %s", snd_strerror(err));
      break;
    }
    if (device < 0)
      break;
    StringList p = list_device(ctl, card, device, mode);
    ports.insert(ports.end(), p.begin(), p.end());
  }
  snd_ctl_close(ctl);

  return ports;
}

ALSADriver::StringList ALSADriver::device_list(snd_rawmidi_stream_t mode)
{
  int card, err;
  StringList ports;
  
  card = -1;
  if ((err = snd_card_next(&card)) < 0) {
    error("cannot determine card number: %s", snd_strerror(err));
    return ports;
  }
  if (card < 0) {
    error("no sound card found");
    return ports;
  }
  do {
    StringList p = list_card_devices(card, mode);
    ports.insert(ports.end(), p.begin(), p.end());
    if ((err = snd_card_next(&card)) < 0) {
      error("cannot determine card number: %s", snd_strerror(err));
      break;
    }
  } while (card >= 0);

  return ports;
}

ALSADriver::StringList ALSADriver::getMidiInputPorts()
{
  return device_list(SND_RAWMIDI_STREAM_INPUT);
}

ALSADriver::StringList ALSADriver::getMidiOutputPorts()
{
  return device_list(SND_RAWMIDI_STREAM_OUTPUT);
}

void ALSADriver::connect(string midiInputPort, string midiOutputPort)
{
  int err = snd_rawmidi_open(&fd_in, &fd_out, midiInputPort.c_str(), 0);
  if (err != 0) {
    throw MidiException("Failed to open midi port.", errno);
  }
}

void ALSADriver::disconnect()
{
  snd_rawmidi_close(fd_in);
  snd_rawmidi_close(fd_out);
}

void ALSADriver::send(Bytes bytes)
{
  unsigned char buffer[bytes.size()];
  int n;
  Bytes::iterator i;

  for(n = 0, i = bytes.begin(); i != bytes.end(); i++, n++) {
    buffer[n] = (*i);
  }

  if (snd_rawmidi_write(fd_out, buffer, bytes.size()) < 0) {
    throw MidiException("Failed to write to midi output port.", errno);
  }
}

void ALSADriver::receive(Bytes& bytes)
{
  unsigned char byte[4096];
  int n;

  int read = 0;
  int npfds, time = 0;
  struct pollfd *pfds;
  
  snd_rawmidi_nonblock(fd_in, 1);
  npfds = snd_rawmidi_poll_descriptors_count(fd_in);
  pfds = (pollfd*)alloca(npfds * sizeof(struct pollfd));
  snd_rawmidi_poll_descriptors(fd_in, pfds, npfds);

  for (;;) {
    unsigned char buf[256];
    int i, length;
    unsigned short revents;
    int err = 0;

    err = poll(pfds, npfds, 10);
    if (err < 0 && errno == EINTR)
      break;
    if (err < 0) {
      error("poll failed: %s", strerror(errno));
      break;
    }
    if (err == 0) {
      break;
    }
    if ((err = snd_rawmidi_poll_descriptors_revents(fd_in, pfds, npfds, &revents)) < 0) {
      error("cannot get poll events: %s", snd_strerror(errno));
      break;
    }
    if (revents & (POLLERR | POLLHUP))
      break;
    if (!(revents & POLLIN))
      continue;
    err = snd_rawmidi_read(fd_in, buf, sizeof(buf));
    if (err == -EAGAIN)
      continue;
    if (err < 0) {
      error("cannot read from port: %s", snd_strerror(err));
      break;
    }
    memcpy(&byte[read], buf, err);
    read += err;
    time = 0;
  }

  bytes.clear();
  
  for (int i = 0; i < read; i++) {
    inputBuffer.push_back(byte[i]);
    if (byte[i] == SYSEX_END) {
      bytes = inputBuffer;
      inputBuffer.clear();
    }
  }
}

#endif // ALSA
