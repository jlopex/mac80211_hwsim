/*
 ============================================================================
 Name        : ieee80211_hdr.h
 Author      : Javier Lopez
 Version     :
 Copyright   : 
 Description :
 ============================================================================
 */


#ifndef IEEE80211_HDR_H_
#define IEEE80211_HDR_H_

struct ieee80211_hdr { 
        unsigned char frame_control[2]; 
       	unsigned char duration_id[2]; 
        unsigned char addr1[6]; 
        unsigned char addr2[6]; 
        unsigned char addr3[6];                
        unsigned char seq_ctrl[2]; 
        unsigned char addr4[6]; 
};

#endif /* IEEE80211_HDR */
