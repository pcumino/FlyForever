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
//*     Module:Software-Defined UAVNet architecture (SD-UAVNet)           *
//*                                                                                                                *
//*     Ref.: D. Rosario, Z. Zhao, T. Braun, E. Cerqueira, A. Santos, and Z. Li, “Assessment of a Robust           *
//*     Opportunistic Routing for Video Transmission in Dynamic Topologies” in Proceedings of the IFIP Wireless    *
//*     Days conference (WD'13). Valencia, Spain, November 13-15, 2013                                             *
//*                                                                                                                *
//*  	Version: 2.0                                                                                               *
//*  	Authors: Denis do Rosário <denis@ufpa.br>                                                                  *
//*                                                                                                                *
//*****************************************************************************************************************/

package castalia.node.communication.routing.FlyForever;

simple FlyForeverRouting like castalia.node.communication.routing.iRouting {
parameters:
	bool collectTraceInfo = default (false);
	bool collectOutputInfo = default (false);
	int maxNetFrameSize = default (0);		//bytes;
	int netDataFrameOverhead= default (10);		//bytes;
	int netBufferSize = default (32);		//number of messages
	string destinationAddress = default ("0");	//Destination addres
	string controllerAddress = default ("0");
	double beaconInterval = default (1);	//Destination addres
	double timeToMoveMax = default (10);
	double minEnergy = default (0.2);
	bool isSource = default (false);		//Used to known if the current node is source
	
gates: 
	output toCommunicationModule;
	output toMacModule;
	input fromCommunicationModule;
	input fromMacModule;
	input fromCommModuleResourceMgr;
}

