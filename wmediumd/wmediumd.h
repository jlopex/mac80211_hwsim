/*
 *	wmediumd, wireless medium simulator for mac80211_hwsim kernel module
 *	Copyright (C) 2011  Javier Lopez (jlopex@gmail.com)
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version 2
 *	of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program; if not, write to the Free Software
 *	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 
 *	02110-1301, USA.
 */

#ifndef WMEDIUMD_H_
#define WMEDIUMD_H_

#define HWSIM_TX_CTL_REQ_TX_STATUS	1
#define HWSIM_TX_CTL_NO_ACK		(1 << 1)
#define HWSIM_TX_STAT_ACK		(1 << 2)

#define HWSIM_CMD_REGISTER 1
#define HWSIM_CMD_FRAME 2
#define HWSIM_CMD_TX_INFO_FRAME 3

#define HWSIM_ATTR_ADDR_RECEIVER 1
#define HWSIM_ATTR_ADDR_TRANSMITTER 2
#define HWSIM_ATTR_FRAME 3
#define HWSIM_ATTR_FLAGS 4
#define HWSIM_ATTR_RX_RATE 5
#define HWSIM_ATTR_SIGNAL 6
#define HWSIM_ATTR_TX_INFO 7
#define HWSIM_ATTR_COOKIE 8
#define HWSIM_ATTR_MAX 8
#define VERSION_NR 1

struct hwsim_tx_rate {
        signed char idx;
        unsigned char count;
};

#endif /* WMEDIUMD_H_ */
