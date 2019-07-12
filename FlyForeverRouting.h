//******************************************************************************************************************
//*     Copyright (c) 2013. Federal University of Para (UFPA), Brazil and                                          *
//*                         University of Bern (UBern), Switzerland                                                *
//*     Developed by Research Group on Computer Network and Multimedia Communication (GERCOM) of UFPA              *
//*     in collaboration to Communication and Distributed Systems (CDS) research group of UBern.                   *
//*     All rights reserved                                                                                        *
//*                                                                                                                *
//*     Permission to use, copy, modify, and distribute this protocol and its documentation for any purpose,       *
//*     without fee, and without written agreement is hereby granted, provided that the above copyright notice,    *
//*     and the author appear in all copies of this protocol.                                                      *
//*                                                                                                                *
//*     Module: Software-Defined UAVNet architecture (SD-UAVNet)            *
//*                                                                                                                *
//*     Ref.: D. Rosario, Z. Zhao, T. Braun, E. Cerqueira, A. Santos, and Z. Li, “Assessment of a Robust           *
//*     Opportunistic Routing for Video Transmission in Dynamic Topologies” in Proceedings of the IFIP Wireless    *
//*     Days conference (WD'13). Valencia, Spain, November 13-15, 2013                                             *
//*                                                                                                                *
//*  	Version: 2.0                                                                                               *
//*  	Authors: Denis do Rosário <denis@ufpa.br>                                                                  *
//*                                                                                                                *
//*****************************************************************************************************************/

#ifndef _FlyForever_ROUTING_H_
#define _FlyForever_ROUTING_H_

#include <cmath>
#include <queue>
#include <vector>
#include <omnetpp.h>
#include <algorithm>
#include <pthread.h>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include "WirelessChannel.h"
#include "Radio.h"
#include "VirtualMac.h"
#include "VirtualRouting.h"
#include "FlyForeverRoutingFrame_m.h"
#include "VirtualMobilityManager.h"

using namespace std;

enum FlyForeverRoutingTimers {
	FlyForever_CLEAN_BUFFER		= 1,
	FlyForever_STATISTICS		= 2,
	FlyForever_SEND_BEACON		= 3,
	FlyForever_DECISION		= 4,
	FlyForever_DECISIONRN		= 5,
	FlyForever_SENDINFO		= 6,
	FlyForever_CHECKACK		= 7,
	FlyForever_ROUTE_STATISTICS	= 8,
	FlyForever_ALIVE		= 9,
	FlyForever_UPDATE_ROUTE		= 10,
};

enum FlyForeverRoutingStatistics {
	FlyForever_STATISTICS_RECEIVED_PACKET	= 2,
	FlyForever_STATISTICS_UNICAST_PACKET	= 3,
	FlyForever_STATISTICS_MOS		= 10,
	FlyForever_STATISTICS_ILOST		= 11,
	FlyForever_STATISTICS_PLOST		= 12,
	FlyForever_STATISTICS_BLOST		= 13,
	FlyForever_STATISTICS_FRAMES		= 14,
	FlyForever_STATISTICS_ROUTE		= 15,
};

struct pktList{
	int seqNumber;
	queue <cPacket *> bufferPkt;
};

struct replacementInfo{
	int nodeId;
	int nextHop;
	int source;
	int destination;
	int line;
	FlyForeverLocationInfo location;
	bool ack;
	bool replace;
	int lastReceivedPacket;
};

struct routeCInfo{
	int nodeId;
	int nextHop;
	int source;
	int destination;
	FlyForeverLocationInfo idealLocation;
	FlyForeverLocationInfo selfLocation;
	bool ack;
	bool replace;
	int lastReceivedPacket;
};

struct nodeCInfo{
	double RE;
	double speed;
	double avg;
	double time;
	double timeMove;
	int id;
	int trajectory;
	FlyForeverLocationInfo selfLocation;
	bool selected;
	bool replacement;
};

struct eventInfo{
	FlyForeverLocationInfo eventLocation;
	double time;
};

struct controllerInfo{
	FlyForeverLocationInfo controllerLocation;
	int controllerId;
};

struct requestData{
	double time;
	double frames;
	int nodeId;
	int videoId;
	int nRecPkts;
	int hopSum;
};

class FlyForeverRouting : public VirtualRouting {
 protected:
	// The .ned file's parameters
	string destinationAddress;
	string controllerAddress;
	double beaconInterval;
	double timeToMoveMax;
	double minEnergy;

	//to LinGO protocol
	VirtualMobilityManager* mobilityModule;
	VirtualMobilityManager* sinkMobilityModule;
	VirtualMobilityManager* sinkMobilityModule1;
	VirtualMac *macModule;
	WirelessChannel *wirelessModule;
	ResourceManager *srceRerouceModuce;
	ResourceManager *energy;
	NodeLocation location;
	FlyForeverLocationInfo currentLocation;
	FlyForeverLocationInfo destinationLocation;
	FlyForeverLocationInfo controllerLocation;
	FlyForeverLocationInfo lastLocation;
	FlyForeverLocationInfo movingLocation;
	FlyForeverLocationInfo idealLastHopLocation;
	FlyForeverLocationInfo idealLocation;
	vector <pktList> buffer;
	vector <routeCInfo> route;
	vector <nodeCInfo> nodes;
	vector <replacementInfo> replaceList;
	vector <replacementInfo> rechargeList;
	vector <eventInfo> eventList;
	vector <controllerInfo> controllerList;

	double tStart;
	double tMax;
	double sensedValue;
	double sim_time_limit;

	//to collect statistics
	vector <requestData> videoStatistics;
	void statistics(int, int, int, int, double, int);
	int statisticNext;
	int statisticLast;
	int statisticFistPkt;
	int statisticLastPkt;
	int statisticQtdPkts;
	double statisticDuration;
	int statisticIdVideo;
	int statisticIdSource;
	bool print;
	double time;
	int idEvent;

	void nodeRecharging(int, int, FlyForeverLocationInfo);
	void nodeReplacement(int, int);
	void sendAdv();
	double getResidualEnergy();
	double getInitialEnergy();
	double getEnergyPerPkt();
	double getRadioRange();
	double getTxPower();
	double getSensitivity();
	void loadEventList();
	void loadControlerList();
	
	FlyForeverLocationInfo getGCSlocation();
	string getControllerId(FlyForeverLocationInfo);
	int getControllerIndex(FlyForeverLocationInfo);
	FlyForeverLocationInfo getCurrentLocation ();
	double computeDistance(FlyForeverLocationInfo, FlyForeverLocationInfo);
	void SNbackboneMode(cPacket *, int);
	void RNreceivedPktBackbone(FlyForeverRoutingPacket *);
	void DNreceivedPkt(cPacket *);
	void dropAlgorithm(FlyForeverRoutingPacket *);
	bool searchBuffer(int);
	FlyForeverLocationInfo computeIdealLocation();
	void startup();
	void selectSource();
	void selectRelay();
	virtual void finish();
	void fromApplicationLayer(cPacket *, const char *);
	void fromMacLayer(cPacket *, int, double, double);
	void timerFiredCallback(int);
	void multipleTimerFiredCallback(int, int);
	void processBufferedPacket(string);
	void setRadio_Sleep(double delay=0.0);
	void setRadio_Listen(double delay=0.0);
        void setRadio_TXPowerLevel(int, double);
};

bool FlyForever_sort_buffer(pktList a, pktList b);
bool FlyForever_sort_node(nodeCInfo a, nodeCInfo b);
bool FlyForever_sort_time(nodeCInfo a, nodeCInfo b);

#endif
