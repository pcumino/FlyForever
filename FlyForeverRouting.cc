
#include "FlyForeverRouting.h"

Define_Module(FlyForeverRouting);

//================================================================
//    finish
//================================================================
void FlyForeverRouting::finish() {
	buffer.clear();
	route.clear();
	nodes.clear();
	replaceList.clear();
	rechargeList.clear();
	eventList.clear();
	controllerList.clear();
	cOwnedObject *Del=NULL;
	int OwnedSize = this->defaultListSize();
	for(int i=0; i<OwnedSize; i++){
		Del = this->defaultListGet(0);
		this->drop(Del);
		delete Del;
	}
}

//================================================================
//    startup
//================================================================
void FlyForeverRouting::startup() {
	destinationAddress = par("destinationAddress").stringValue();
	controllerAddress = par("controllerAddress").stringValue();
	beaconInterval = par("beaconInterval").doubleValue();
	timeToMoveMax = par("timeToMoveMax").doubleValue();
	minEnergy = par("minEnergy").doubleValue();

	string s = ev.getConfig()->getConfigValue("sim-time-limit");
	s = s.erase(s.find('s'));
   	sim_time_limit = atof(s.c_str());

	wirelessModule = check_and_cast <WirelessChannel*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("wirelessChannel"));
	mobilityModule = check_and_cast <VirtualMobilityManager*>(getParentModule()->getParentModule()->getSubmodule("MobilityManager"));
	location = mobilityModule->getLocation();
	self = getParentModule()->getParentModule()->getIndex();
	radioModule = check_and_cast <Radio*>(getParentModule()->getSubmodule("Radio"));
	macModule = check_and_cast <VirtualMac*>(getParentModule()->getSubmodule("MAC"));

	sinkMobilityModule = check_and_cast <VirtualMobilityManager*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",atoi(destinationAddress.c_str()))->getSubmodule("MobilityManager"));
	destinationLocation.x = sinkMobilityModule->getLocation().x;
	destinationLocation.y = sinkMobilityModule->getLocation().y;
	
	sinkMobilityModule1 = check_and_cast <VirtualMobilityManager*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",atoi(controllerAddress.c_str()))->getSubmodule("MobilityManager"));
	controllerLocation.x = sinkMobilityModule1->getLocation().x;
	controllerLocation.y = sinkMobilityModule1->getLocation().y;

	srceRerouceModuce = check_and_cast <ResourceManager*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",1)->getSubmodule("ResourceManager"));

	loadControlerList();
	loadEventList();
	time = 0;
	print = false;
	if (self!=atoi(controllerAddress.c_str()) && self!=atoi(destinationAddress.c_str()) && self < 20){
		int tmp = rand()%100;
		double tmpTime = tmp / 100.0;
		setTimer(FlyForever_SEND_BEACON, 1.5 + tmpTime);
	}
	if (self==atoi(controllerAddress.c_str())){
		double time = eventList[idEvent].time - SIMTIME_DBL(simTime());
		setTimer(FlyForever_DECISION, time);
	}
	if (self == atoi(destinationAddress.c_str())){
		setTimer(FlyForever_STATISTICS, sim_time_limit - 0.5);
	} else if (self == atoi(controllerAddress.c_str())){
		trace() << "Software-Defined UAVNet architecture (SD-UAVNet)\n\n";
		setTimer(FlyForever_ALIVE, 10-0.05);
	} else {
		setTimer(FlyForever_STATISTICS, sim_time_limit - 0.8);
	}
}

//================================================================
//    fromApplicationLayer
//================================================================
void FlyForeverRouting::fromApplicationLayer(cPacket *pkt, const char *dstAddr){
	switch (pkt->getKind()) {
		case MULTIMEDIA_PACKET:{
			
			ApplicationPacket *appPkt = check_and_cast <ApplicationPacket*>(pkt);
			trace() << "SN_" << self << " received the " << appPkt->getInfo().frameType << "-frame " << appPkt->getFrame() << " fragmented into packets " << appPkt->getIdFrame() << " for video id " << appPkt->getIdVideo() << " with " << appPkt->getByteLength() << " bytes from App Layer";
			trace() << "SN_" << self << " location (" << getCurrentLocation().x << "," << getCurrentLocation().y << ")" << " => Ideal Location (" << route[0].idealLocation.x << "," << route[0].idealLocation.y << ")";
			trace() << "Distance to ideal " << computeDistance(route[0].idealLocation, getCurrentLocation());
			trace() << "Distance to DN " << computeDistance(route[0].idealLocation, destinationLocation);
			if (route.size() > 0)
				SNbackboneMode(pkt, appPkt->getIdVideo());
			break;
		}
		case APPLICATION_STATISTICS_MESSAGE:{
			ApplicationPacket *appPkt = check_and_cast <ApplicationPacket*>(pkt);
			break;
		}
		case SCALAR_PACKET:{
			ApplicationPacket *appPkt = check_and_cast <ApplicationPacket*>(pkt);
			sensedValue = appPkt->getData();
			//trace() << "received an data from app " << sensedValue;
			break;
		}
	}
}

//================================================================
//    fromMacLayer
//================================================================
void FlyForeverRouting::fromMacLayer(cPacket *pkt, int srcMacAddress, double RSSI, double LQI){
    switch (pkt->getKind()) {
		case NETWORK_LAYER_PACKET:{
			FlyForeverRoutingPacket *netPacket = dynamic_cast <FlyForeverRoutingPacket*>(pkt);
			if (!netPacket)
				return;
			switch (netPacket->getFlyForeverRoutingPacketKind()) {
				case FlyForever_MULTIMEDIA_PKT:{
					if (self == atoi(netPacket->getDestination())){
						DNreceivedPkt(pkt);
					} else
						RNreceivedPktBackbone(netPacket);
					break;
				}
				case FlyForever_ADV_MSG:{
					if (self == atoi(controllerAddress.c_str())){
						//trace() << "Conroller received an adv msg from F_" << netPacket->getSource() << " with RE " << netPacket->getRE() << " J (" << netPacket->getRE()/getInitialEnergy() << " %), located at (" << netPacket->getCurrentLocation().x << "," << netPacket->getCurrentLocation().y << ") moving at " << netPacket->getSpeed() << "m/s\n";
						int value = -1;
						for (int i=1; i<route.size(); i++){
							if (atoi(netPacket->getSource()) == route[i].nodeId){
								value = i;
								break;
							}
						}
						if (value != -1 ){
							int indexReplacement = -1;
							for (int i=0; i<replaceList.size(); i++){
								if (atoi(netPacket->getSource()) == replaceList[i].nodeId){
									indexReplacement = i;
									break;
								}
							}
							double REt = netPacket->getRE()/getInitialEnergy();

							double distanceToGCS = computeDistance(netPacket->getCurrentLocation(), getGCSlocation());
							double EtoGCS = distanceToGCS * ((0.95 * (mobilityModule->getMinSpeed() * mobilityModule->getMinSpeed())) - (20.4 * mobilityModule->getMinSpeed()) + 130);
							double EtoGCSsafetyMargin = 1.2 * EtoGCS;
							if(netPacket->getRE() <= EtoGCSsafetyMargin && indexReplacement == -1){
								trace() << "Conroller received an adv msg from F_" << netPacket->getSource() << " with RE " << netPacket->getRE() << " J (" << REt << " %), located at (" << netPacket->getCurrentLocation().x << "," << netPacket->getCurrentLocation().y << ") moving at " << netPacket->getSpeed() << "m/s";
								/*trace() << "current location " << netPacket->getCurrentLocation().x << ", "<< netPacket->getCurrentLocation().y;
								trace() << "cgs location " << getGCSlocation().x << ", "<< getGCSlocation().y;
								trace() << "distanceToGCS " << distanceToGCS;
								trace() << "min speed " <<  mobilityModule->getMinSpeed();
								trace() << "EtoGCS " << EtoGCS;
								trace() << "min energy " << EtoGCSsafetyMargin;
								trace() << "current energy " << netPacket->getRE();*/
								trace() << "Nó [" << netPacket->getSource() << "] atingiu limite mínimo de energia para chegar ao GCS: " << netPacket->getRE() << "\t " << EtoGCSsafetyMargin;
								nodeReplacement(value, atoi(netPacket->getSource()));
							}
						}

						value = -1;
						for (int i=0; i<replaceList.size(); i++){
							if (atoi(netPacket->getSource()) == replaceList[i].nodeId){
								value = i;
								break;
							}
						}
						if (value != -1 ){
							double REt = netPacket->getRE()/getInitialEnergy();
							trace() << "[nodeRecharging] FlyForever_REPO_MSG";
							trace() << "Conroller received an adv msg from R_" << netPacket->getSource() << " with RE " << netPacket->getRE() << " J (" << REt << " %), located at (" << netPacket->getCurrentLocation().x << "," << netPacket->getCurrentLocation().y << ") moving at " << netPacket->getSpeed() << "m/s";
							double distance = computeDistance(replaceList[value].location, netPacket->getCurrentLocation());
							trace() << "dist " << distance;
							if (distance < 15){
								trace() << "update routing table line number " << replaceList[value].line;
								int routeId = replaceList[value].line;
								int nodeToRecharge = route[routeId-1].nextHop;
								FlyForeverLocationInfo loc = route[routeId-1].selfLocation;
								route[routeId].nodeId = replaceList[value].nodeId;
								route[routeId-1].nextHop = replaceList[value].nodeId;
								route[routeId].replace = false;
								//trace()<<"srcId\tdstId\tnodeId\tnextId\tLoc.x\tLoc.y\tack\treplace";
								for (int i=routeId-1; i <= routeId; i++){
									char buff[256];
									string sNodeID;
									sprintf(buff, "%i", route[i].nodeId);
									sNodeID.assign(buff);
									string sNextId;
									sprintf(buff, "%i", route[i].nextHop);
									sNextId.assign(buff);
									route[i].ack = false;

									FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
									advMsg->setByteLength(netDataFrameOverhead);
									if (i == 0)
										advMsg->setFlyForeverRoutingPacketKind(FlyForever_SN_MSG);
									else
										advMsg->setFlyForeverRoutingPacketKind(FlyForever_RN_MSG);
									advMsg->setSource(SELF_NETWORK_ADDRESS);
									advMsg->setDestination(sNodeID.c_str());
									advMsg->setNextHop(sNodeID.c_str());
									advMsg->setNextHopRoute(sNextId.c_str());
									advMsg->setIdealLocation(route[i].idealLocation);
									advMsg->setSourceRoute(route[i].source);
									advMsg->setReplacement(false);
									toMacLayer(advMsg, resolveNetworkAddress(advMsg->getDestination()));
									trace() << "C_" << self << " is sending a msg to node " << advMsg->getDestination() << " with next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")";
								}
								int indexRoute = -1;
								for (int i=0; i<route.size(); i++){
									if (atoi(netPacket->getSource()) == route[i].nodeId){
										indexRoute = i;
										break;
									}
								}
								replacementInfo temp;
								temp.source = atoi(netPacket->getSource());
								temp.nodeId = atoi(netPacket->getSource());
								temp.location = netPacket->getCurrentLocation();
								temp.ack = false;
								temp.replace = false;
								rechargeList.push_back(temp);
								nodes[indexRoute].replacement = true;
								nodeRecharging(value, nodeToRecharge, loc);
								setTimer(FlyForever_CHECKACK, 0.1);
								replaceList.clear();
							}
						}

						int index = -1;
						for (int i=0; i<nodes.size(); i++){
							if (nodes[i].id == atoi(netPacket->getSource())){
								index = i;
								break;
							}
						}
						if (index == -1){
							nodeCInfo temp;
							temp.id = atoi(netPacket->getSource());
							nodes.push_back(temp);
							int i = nodes.size() - 1;
							nodes[i].time = SIMTIME_DBL(simTime());
							nodes[i].id = atoi(netPacket->getSource());
							nodes[i].RE = netPacket->getRE();
							nodes[i].selfLocation = netPacket->getCurrentLocation();
							nodes[i].speed = netPacket->getSpeed();
							nodes[i].trajectory = netPacket->getTraject();
							nodes[i].selected = false;
							nodes[i].replacement = false;

						} else{
							int indexRoute = -1;
							for (int i=0; i<route.size(); i++){
								if (atoi(netPacket->getSource()) == route[i].nodeId){
									indexRoute = i;
									break;
								}
							}
							int indexReplacement = -1;
							for (int i=0; i<rechargeList.size(); i++){
								if (atoi(netPacket->getSource()) == rechargeList[i].nodeId){
									indexReplacement = i;
									break;
								}
							}
							
							if (netPacket->getRE() > nodes[index].RE){
								trace() << "node " << netPacket->getSource() << " replaced the batery";
								nodes[index].replacement = false;
							}
							nodes[index].time = SIMTIME_DBL(simTime());
							nodes[index].id = atoi(netPacket->getSource());
							nodes[index].RE= netPacket->getRE();
							nodes[index].selfLocation = netPacket->getCurrentLocation();
							nodes[index].speed = netPacket->getSpeed();
							nodes[index].trajectory = netPacket->getTraject();

							double distanceToGCS = computeDistance(netPacket->getCurrentLocation(), getGCSlocation());
							double EtoGCS = distanceToGCS * ((0.95 * (mobilityModule->getMinSpeed() * mobilityModule->getMinSpeed())) - (20.4 * mobilityModule->getMinSpeed()) + 130);
							double EtoGCSsafetyMargin = 1.2 * EtoGCS;
							if(netPacket->getRE() <= EtoGCSsafetyMargin && indexRoute == -1 && indexReplacement == -1){
								trace() << "node update";
								trace() << "Conroller received an adv msg from F_" << netPacket->getSource() << " with RE " << netPacket->getRE() << " J (" << netPacket->getRE()/getInitialEnergy() << " %), located at (" << netPacket->getCurrentLocation().x << "," << netPacket->getCurrentLocation().y << ") moving at " << netPacket->getSpeed() << "m/s";
								/*trace() << "current location " << netPacket->getCurrentLocation().x << ", "<< netPacket->getCurrentLocation().y;
								trace() << "cgs location " << getGCSlocation().x << ", "<< getGCSlocation().y;
								trace() << "distanceToGCS " << distanceToGCS;
								trace() << "min speed " <<  mobilityModule->getMinSpeed();
								trace() << "EtoGCS " << EtoGCS;
								trace() << "min energy " << EtoGCSsafetyMargin;
								trace() << "current energy " << netPacket->getRE();*/
								trace() << "Nó [" << netPacket->getSource() << "] is within the energy limit to reach the GCS: " << netPacket->getRE() << "\t " << EtoGCSsafetyMargin;

								replacementInfo temp;
								temp.source = atoi(netPacket->getSource());
								temp.nodeId = atoi(netPacket->getSource());
								temp.location = netPacket->getCurrentLocation();
								temp.ack = false;
								temp.replace = false;
								rechargeList.push_back(temp);
								nodes[indexRoute].replacement = true;

								char buff[256];
								string sNodeID;
								sprintf(buff, "%i", temp.nodeId);
								sNodeID.assign(buff);
								string sNextHopId = getControllerId(netPacket->getCurrentLocation());

								FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
								advMsg->setByteLength(netDataFrameOverhead);
								advMsg->setFlyForeverRoutingPacketKind(FlyForever_REPO_MSG);
								advMsg->setSource(SELF_NETWORK_ADDRESS);
								advMsg->setDestination(sNodeID.c_str());
								if (atoi(sNextHopId.c_str()) == self)
									advMsg->setNextHop(sNodeID.c_str());
								else
									advMsg->setNextHop(sNextHopId.c_str());
								advMsg->setIdealLocation(getGCSlocation());
								advMsg->setRecharging(true);
								toMacLayer(advMsg, resolveNetworkAddress(advMsg->getNextHop()));
								setTimer(FlyForever_CHECKACK, 0.1);
								if (atoi(sNextHopId.c_str()) == self)
									trace() << "C_" << self << " is sending a energy replacement msg to node_" << advMsg->getDestination() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")";
								else
									trace() << "C_" << self << " is sending a energy replacement msg to node_" << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")";

							}
						}
					} else {
						FlyForeverRoutingPacket *dupPacket = netPacket->dup();
						dupPacket->setNextHop(controllerAddress.c_str());
						toMacLayer(dupPacket, resolveNetworkAddress(dupPacket->getNextHop()));
						//trace() << "[Conroller forwarder]_" << self << " is forwarding an adv msg to the controller_" << dupPacket->getDestination() << " via controller_" << dupPacket->getNextHop() << " from node " << dupPacket->getSource();
					}
					break;
				}
				case FlyForever_ACK_MSG:{
					if (self == atoi(netPacket->getDestination())){
						trace() << "Conroller received an ack msg from F_" << netPacket->getSource();
						int index = -1;
						if (!netPacket->getReplacement()){
							for (int i=0; i<route.size(); i++){
								if (route[i].nodeId == atoi(netPacket->getSource())){
									index = i;
									break;
								}
							}
							if (index != -1){
								route[index].ack = true;
								trace() << "Updated \n";
							} 
						} else {
							index = -1;
							for (int i=0; i<rechargeList.size(); i++){
								if (rechargeList[i].nodeId == atoi(netPacket->getSource())){
									index = i;
									break;
								}
							}
							if (index != -1)
								rechargeList.erase(rechargeList.begin()+index);
							index = -1;
							for (int i=0; i<replaceList.size(); i++){
								if (replaceList[i].nodeId == atoi(netPacket->getSource())){
									index = i;
									break;
								}
							}
							if (index != -1){
								replaceList[index].ack = true;
								trace() << "Updated \n";
							} 
						}
					} else {
						FlyForeverRoutingPacket *dupPacket = netPacket->dup();
						dupPacket->setNextHop(controllerAddress.c_str());
						toMacLayer(dupPacket, resolveNetworkAddress(dupPacket->getNextHop()));
						trace() << "[Conroller forwarder]_" << self << " is forwarding an ack msg to the controller_" << dupPacket->getDestination() << " via controller_" << dupPacket->getNextHop();
					}
					break;
				}
				case FlyForever_REPO_MSG:{
					if (self == atoi(netPacket->getDestination())){
						trace() << "[nodeRecharging] FlyForever_REPO_MSG";
						ApplicationPacket *topology = new ApplicationPacket("setup", MOBILE_GCS_RECHARGE);
						topology->setX(netPacket->getIdealLocation().x); // definir depois o local do evento
						topology->setY(netPacket->getIdealLocation().y); 
						toApplicationLayer(topology);

						int index = getControllerIndex(getCurrentLocation());
						string sNextHopId = getControllerId(getCurrentLocation());
						if(computeDistance(getCurrentLocation(),controllerList[index].controllerLocation) <= 60)
							setRadio_TXPowerLevel(12,0);
						else
							setRadio_TXPowerLevel(30,0);

						FlyForeverRoutingPacket *ackMsg = new FlyForeverRoutingPacket("Node is sending an ack msg", NETWORK_LAYER_PACKET);
						ackMsg->setByteLength(netDataFrameOverhead);
						ackMsg->setFlyForeverRoutingPacketKind(FlyForever_ACK_MSG);
						ackMsg->setSource(SELF_NETWORK_ADDRESS);
						ackMsg->setDestination(controllerAddress.c_str());
						ackMsg->setNextHop(sNextHopId.c_str());
						ackMsg->setReplacement(true);
						toMacLayer(ackMsg, resolveNetworkAddress(ackMsg->getNextHop()));
						setRadio_TXPowerLevel(12,0.05);
						trace() << "node_" << self << " is sending an ack msg to C_" << ackMsg->getDestination() << " via C_" << ackMsg->getNextHop() <<"\n";
					} else {
						FlyForeverRoutingPacket *dupPacket = netPacket->dup();
						dupPacket->setNextHop(netPacket->getDestination());
						toMacLayer(dupPacket, resolveNetworkAddress(dupPacket->getNextHop()));
						trace() << "[Conroller forwarder]_" << self << " is forwarding a route msg to node_" << dupPacket->getDestination();
					}
					break;
				}

				case FlyForever_SN_MSG:{
					if (self == atoi(netPacket->getDestination())){
						if (replaceList.size()!=0){
							if(replaceList[0].nodeId==self){
								trace() << "Node " << self << " received a msg from controller to replace SN";
							}	
						} else{
							trace() << "Node " << self << " received a msg from controller to become SN";
						}

						if (route.size() == 0){
							routeCInfo temp;
							temp.nodeId = atoi(netPacket->getDestination());
							temp.idealLocation = netPacket->getIdealLocation();
							temp.nextHop = atoi(netPacket->getNextHopRoute());
							temp.source = netPacket->getSourceRoute();
							temp.destination = atoi(destinationAddress.c_str());
							route.push_back(temp);
							trace() << "Node " << self << " become Source Node and start the video after " << netPacket->getStartVideo();

							ApplicationPacket *topology = new ApplicationPacket("setup", MOBILE_MESSAGE);
							topology->setEventType(1);// ver os tipos na APP
							topology->setX(netPacket->getIdealLocation().x); // definir depois o local do evento
							topology->setY(netPacket->getIdealLocation().y); 
							topology->setStartVideo(netPacket->getStartVideo());
							toApplicationLayer(topology);
							setTimer(FlyForever_UPDATE_ROUTE, 12 + netPacket->getStartVideo());
						} else {
							int index = -1;
							for (int i=0; i < route.size(); i++){
								if (route[i].source == netPacket->getSourceRoute()){
									index = i;
									break;
								}
							}
							if (index == -1){
								trace() << "update the route";
								index = route.size() - 1;
								route[index].source = netPacket->getSourceRoute();
								route[index].destination = atoi(destinationAddress.c_str());
								route[index].nodeId = atoi(netPacket->getDestination());
								route[index].nextHop = atoi(netPacket->getNextHopRoute());
								route[index].idealLocation = netPacket->getIdealLocation();

								ApplicationPacket *topology = new ApplicationPacket("setup", MOBILE_MESSAGE);
								topology->setEventType(1);// ver os tipos na APP
								topology->setX(netPacket->getIdealLocation().x); // definir depois o local do evento
								topology->setY(netPacket->getIdealLocation().y); 
								topology->setStartVideo(netPacket->getStartVideo());
								toApplicationLayer(topology);
								setTimer(FlyForever_UPDATE_ROUTE, 12 + netPacket->getStartVideo());
							}
						}

						trace() << "routing table ";
						trace()<<"src\tdst\tId\tnext\tLoc.x\tLoc.y\tack\tStart";
						for (int i=0; i < route.size(); i++)
							trace() << route[i].source << "\t" << route[i].destination << "\t" << route[i].nodeId << "\t" << route[i].nextHop << "\t" << route[i].idealLocation.x << "\t" << route[i].idealLocation.y << "\t" << route[i].ack << "\t" << netPacket->getStartVideo();

						int index = getControllerIndex(getCurrentLocation());
						string sNextHopId = getControllerId(getCurrentLocation());
						if(computeDistance(getCurrentLocation(),controllerList[index].controllerLocation) <= 60)
							setRadio_TXPowerLevel(12,0);
						else
							setRadio_TXPowerLevel(30,0);

						FlyForeverRoutingPacket *ackMsg = new FlyForeverRoutingPacket("Node is sending an ack msg", NETWORK_LAYER_PACKET);
						ackMsg->setByteLength(netDataFrameOverhead);
						ackMsg->setFlyForeverRoutingPacketKind(FlyForever_ACK_MSG);
						ackMsg->setSource(SELF_NETWORK_ADDRESS);
						ackMsg->setDestination(controllerAddress.c_str());
						ackMsg->setNextHop(sNextHopId.c_str());
						ackMsg->setReplacement(netPacket->getReplacement());
						toMacLayer(ackMsg, resolveNetworkAddress(ackMsg->getNextHop()));
						setRadio_TXPowerLevel(12,0.05);
						trace() << "node_" << self << " is sending an ack msg to C_" << ackMsg->getDestination() << " via C_" << ackMsg->getNextHop() <<"\n";
					} else {
						FlyForeverRoutingPacket *dupPacket = netPacket->dup();
						dupPacket->setNextHop(netPacket->getDestination());
						toMacLayer(dupPacket, resolveNetworkAddress(dupPacket->getNextHop()));
						trace() << "[Conroller forwarder]_" << self << " is forwarding a route msg to node_" << dupPacket->getDestination();
					}
					break;
				}
				case FlyForever_RN_MSG:{
					if (self == atoi(netPacket->getDestination())){
						if (replaceList.size()!=0){
							if(replaceList[0].nodeId==self){
								trace() << "Node " << self << " received an msg from controller to replace a RN";
							}	
						} else{
							trace() << "Node " << self << " received an msg from controller to become RN";
						}
					
						if (route.size() == 0){
							routeCInfo temp;
							temp.nodeId = atoi(netPacket->getDestination());
							temp.idealLocation = netPacket->getIdealLocation();
							temp.nextHop = atoi(netPacket->getNextHopRoute());
							temp.source = netPacket->getSourceRoute();
							temp.destination = atoi(destinationAddress.c_str());
							route.push_back(temp);	

							ApplicationPacket *topology = new ApplicationPacket("setup", MOBILE_MESSAGE);
							topology->setEventType(2);// ver os tipos na APP
							topology->setX(netPacket->getIdealLocation().x); // definir depois o local do evento
							topology->setY(netPacket->getIdealLocation().y); 
							toApplicationLayer(topology);
							setTimer(FlyForever_UPDATE_ROUTE, 12 + netPacket->getStartVideo());
						} else {
							int index = -1;
							for (int i=0; i < route.size(); i++){
								if (route[i].source == netPacket->getSourceRoute()){
									index = i;
									break;
								}
							}
							if (index == -1){
								trace() << "update the route";
								index = route.size() - 1;
								route[index].source = netPacket->getSourceRoute();
								route[index].destination = atoi(destinationAddress.c_str());
								route[index].nodeId = atoi(netPacket->getDestination());
								route[index].nextHop = atoi(netPacket->getNextHopRoute());
								route[index].idealLocation = netPacket->getIdealLocation();


								ApplicationPacket *topology = new ApplicationPacket("setup", MOBILE_MESSAGE);
								topology->setEventType(2);// ver os tipos na APP
								topology->setX(netPacket->getIdealLocation().x); // definir depois o local do evento
								topology->setY(netPacket->getIdealLocation().y); 
								toApplicationLayer(topology);
								setTimer(FlyForever_UPDATE_ROUTE, 12 + netPacket->getStartVideo());
							}
						}

						trace() << "routing table";
						trace()<<"src\tdst\tId\tnext\tLoc.x\tLoc.y\tack";
						for (int i=0; i < route.size(); i++)
							trace() << route[i].source << "\t" << route[i].destination << "\t" << route[i].nodeId << "\t" << route[i].nextHop << "\t" << route[i].idealLocation.x << "\t" << route[i].idealLocation.y << "\t" << route[i].ack;

						int index = getControllerIndex(getCurrentLocation());
						string sNextHopId = getControllerId(getCurrentLocation());
						if(computeDistance(getCurrentLocation(),controllerList[index].controllerLocation) <= 60)
							setRadio_TXPowerLevel(12,0);
						else
							setRadio_TXPowerLevel(30,0);

						FlyForeverRoutingPacket *ackMsg = new FlyForeverRoutingPacket("Node is sending an ack msg", NETWORK_LAYER_PACKET);
						ackMsg->setByteLength(netDataFrameOverhead);
						ackMsg->setFlyForeverRoutingPacketKind(FlyForever_ACK_MSG);
						ackMsg->setSource(SELF_NETWORK_ADDRESS);
						ackMsg->setDestination(controllerAddress.c_str());
						ackMsg->setNextHop(sNextHopId.c_str());
						ackMsg->setReplacement(netPacket->getReplacement());
						toMacLayer(ackMsg, resolveNetworkAddress(ackMsg->getNextHop()));
						setRadio_TXPowerLevel(12,0.05);
						trace() << "node_" << self << " is sending an ack msg to C_" << ackMsg->getDestination() << " via C_" << ackMsg->getNextHop() <<"\n";
					} else {
						FlyForeverRoutingPacket *dupPacket = netPacket->dup();
						dupPacket->setNextHop(netPacket->getDestination());
						toMacLayer(dupPacket, resolveNetworkAddress(dupPacket->getNextHop()));
						trace() << "[Conroller forwarder]_" << self << " is forwarding a route msg to node_" << dupPacket->getDestination();
					}
					break;
				}
			}
			break;
		}
		case MAC_STATISTICS_MESSAGE:{
			RoutingPacket *netPacket = check_and_cast <RoutingPacket*>(pkt);
			trace() << "FlyForever_CONTENTION_MODE to SN_" << netPacket->getSource() << " - F_" << self << " dropped a packet at MAC layer for SN_" << netPacket->getSource();
			//statistics(atoi(netPacket->getSource()), atoi(destinationAddress.c_str()), netPacket->getIdVideo(), 0, 0, FlyForever_STATISTICS_DROP_MAC);
			break;
		}
	}
}

//================================================================
//    multipleTimerFiredCallback
//================================================================
void FlyForeverRouting::multipleTimerFiredCallback(int index, int nodeId){
	char buff[256];
	string sNodeID;
	sprintf(buff, "%i", nodeId);
	sNodeID.assign(buff);
	switch (index) {
		case FlyForever_CLEAN_BUFFER:{
			trace() << "FlyForever - case: FlyForever_CLEAN_BUFFER of Node " << self << " for SN_" << nodeId << " at time = " << simTime();
			sort(buffer.begin(), buffer.end(), FlyForever_sort_buffer);
			while(buffer.size() > 0){
				for (int i=0; i<buffer.size(); i++){
					FlyForeverRoutingPacket *packet = dynamic_cast <FlyForeverRoutingPacket*>(buffer[i].bufferPkt.front());
					if (atoi(packet->getSource()) == nodeId){
						trace() << "Send pkt number " << buffer[i].seqNumber << " to application";
						toApplicationLayer(decapsulatePacket(buffer[i].bufferPkt.front()));
						buffer.erase(buffer.begin()+i);
						break;
					}
				}
			}
			trace() << "FlyForever - End: FlyForever_CLEAN_BUFFER of Node " << self << " at time = " << simTime() << "\n";
			break;
		}
	}
}

//================================================================
//    timerFiredCallback
//================================================================
void FlyForeverRouting::timerFiredCallback(int index){
	switch (index) {
		case FlyForever_ROUTE_STATISTICS:{
			output() << "\nRoute information";
			output() << "idVideo\tidSrc\tself\tnext\tfirst\tlast\tqtdPkts\tduration"; //TODO: tempo de rota
			//TODO: tempo de rota
			if (!print){
				statisticDuration = SIMTIME_DBL(simTime()) - statisticDuration;
				if(statisticDuration < 15){
					statistics(statisticIdSource, atoi(destinationAddress.c_str()), statisticIdVideo, 0, 0, FlyForever_STATISTICS_ROUTE);
					output() << statisticIdVideo << "\t" << statisticIdSource << "\t" << self << "\t" << statisticNext << "\t" << statisticFistPkt << "\t" << statisticLastPkt << "\t" << statisticQtdPkts << "\t" << statisticDuration;
				}
				print = true;
			}
			//TODO: tempo de rota
			break;
		}
		case FlyForever_ALIVE:{
			time +=10;
			if (self == 20){
				output() << "\nNode Alive";
				output() << "idVideo\ttime\tenergy";
				for (int i=1; i<20; i++){
					energy = check_and_cast <ResourceManager*>(getParentModule()->getParentModule()->getParentModule()->getSubmodule("node",i)->getSubmodule("ResourceManager"));
					if(energy->getRemainingEnergy() > 0){
						output() << i << "\t" 
							 << time << "\t" 
							 << energy->getRemainingEnergy()/energy->getInitialEnergy();
					}
				}
			}
			if (time + 10 < 1203)
				setTimer(FlyForever_ALIVE, 10);
			break;
		}
		case FlyForever_STATISTICS:{
			if (self == atoi(destinationAddress.c_str())){
				output() << "\nTransmitted Videos";
				output() << "time\tidVideo\tidSrc\trecPkts\thops";
				for (int i=0; i< videoStatistics.size(); i++){
					output() << videoStatistics[i].time << "\t" 
						 << videoStatistics[i].videoId << "\t" 
						 << videoStatistics[i].nodeId << "\t" 
						 << videoStatistics[i].nRecPkts << "\t" 
						 << (double) videoStatistics[i].hopSum / videoStatistics[i].nRecPkts;
				}
			} else if (videoStatistics.size() > 0){
				output() << "\nStatistics per node";
				output() << "self\tidVideo\tidSrc\troutes";
				for (int i=0; i<videoStatistics.size(); i++){
					output() << self << "\t" 
						 << videoStatistics[i].videoId << "\t" 
						 << videoStatistics[i].nodeId << "\t" 
						 << videoStatistics[i].hopSum;
				}
			}
			buffer.clear();
			route.clear();
			nodes.clear();
			replaceList.clear();
			rechargeList.clear();
			eventList.clear();
			controllerList.clear();			
			break;
		} 
		case FlyForever_SEND_BEACON:{
			sendAdv();
			setTimer(FlyForever_SEND_BEACON,beaconInterval);
            		break;
		}
		case FlyForever_DECISION:{
			trace() << "FlyForever_DECISION";
			route.clear();
			for (int i=0; i<nodes.size(); i++){
				nodes[i].selected = false;
			}
			selectSource();
			idEvent ++;
			if (idEvent < eventList.size()){
				double time = eventList[idEvent].time - SIMTIME_DBL(simTime());
				setTimer(FlyForever_DECISION, time);
			}
			break;
		}
		case FlyForever_DECISIONRN:{
			 selectRelay();
			break;
		}
		case FlyForever_UPDATE_ROUTE:{
			trace() << "FlyForever_UPDATE_ROUTE";
			route.clear();
			break;
		}
		
		case FlyForever_SENDINFO:{	
			trace() << "routing table";
			trace()<<"src\tdst\tId\tnext\tLoc.x\tLoc.y\tack";
			for (int i=0; i < route.size(); i++){
				trace() << route[i].source << "\t" << route[i].destination << "\t" << route[i].nodeId << "\t" << route[i].nextHop << "\t" << route[i].idealLocation.x << "\t" << route[i].idealLocation.y << "\t" << route[i].ack;
			}
			if (tStart > 1)
				tStart = tStart - 0.5;
			for (int i=0; i<route.size(); i++){
				char buff[256];
				string sNodeID;
				sprintf(buff, "%i", route[i].nodeId);
				sNodeID.assign(buff);
				string sNextId;
				sprintf(buff, "%i", route[i].nextHop);
				sNextId.assign(buff);
				route[i].ack = false;
				string sNextHopId = getControllerId(route[i].selfLocation);

				FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
				advMsg->setByteLength(netDataFrameOverhead);
				if (i == 0)
					advMsg->setFlyForeverRoutingPacketKind(FlyForever_SN_MSG);
				else
					advMsg->setFlyForeverRoutingPacketKind(FlyForever_RN_MSG);
				advMsg->setSource(SELF_NETWORK_ADDRESS);
				advMsg->setDestination(sNodeID.c_str());
				if (atoi(sNextHopId.c_str()) == self)
					advMsg->setNextHop(sNodeID.c_str());
				else
					advMsg->setNextHop(sNextHopId.c_str());
				advMsg->setNextHopRoute(sNextId.c_str());
				advMsg->setIdealLocation(route[i].idealLocation);
				advMsg->setSourceRoute(route[i].source);
				advMsg->setReplacement(false);
				advMsg->setStartVideo(tStart);
				toMacLayer(advMsg, resolveNetworkAddress(advMsg->getNextHop()));
				if (atoi(sNextHopId.c_str()) == self)
					trace() << "C_" << self << " is sending a route msg to node_" << advMsg->getDestination() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ") to start the video after " << tStart << "S";
				else
					trace() << "C_" << self << " is sending a route msg to node_" << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ") to start the video after " << tStart << "S";
			}
			setTimer(FlyForever_CHECKACK, 0.1);
			break;
		}
		case FlyForever_CHECKACK:{
			bool needAck = false;
			for (int i=0; i<route.size(); i++){
				if (route[i].ack == false ){
					needAck=true;
					char buff[256];
					string sNodeID;
					sprintf(buff, "%i", route[i].nodeId);
					sNodeID.assign(buff);
					string sNextId;
					sprintf(buff, "%i", route[i].nextHop);
					sNextId.assign(buff);
					string sNextHopId = getControllerId(route[i].selfLocation);

					FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
					advMsg->setByteLength(netDataFrameOverhead);
					if (i == 0)
						advMsg->setFlyForeverRoutingPacketKind(FlyForever_SN_MSG);
					else
						advMsg->setFlyForeverRoutingPacketKind(FlyForever_RN_MSG);
					advMsg->setSource(SELF_NETWORK_ADDRESS);
					advMsg->setDestination(sNodeID.c_str());
					if (atoi(sNextHopId.c_str()) == self)
						advMsg->setNextHop(sNodeID.c_str());
					else
						advMsg->setNextHop(sNextHopId.c_str());
					advMsg->setNextHopRoute(sNextId.c_str());
					advMsg->setIdealLocation(route[i].idealLocation);
					advMsg->setSourceRoute(route[i].source);
					advMsg->setReplacement(false);
					advMsg->setStartVideo(tStart);
					toMacLayer(advMsg, resolveNetworkAddress(advMsg->getNextHop()));
					if(replaceList.size()>0){
						if (replaceList[0].nodeId==self){
							trace() << "C_" << self << " sending a replace msg to node " << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y  << ") to start the video after " << tStart << "S";
						}			
					} else{
						if (atoi(sNextHopId.c_str()) == self)
							trace() << "C_" << self << " is sending a route msg to node_" << advMsg->getDestination() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ") to start the video after " << tStart << "S";
						else
							trace() << "C_" << self << " is sending a route msg to node_" << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ") to start the video after " << tStart << "S";
					}
					
				}
			}
			for (int i=0; i<rechargeList.size(); i++){
				if (rechargeList[i].ack == false){
					needAck=true;
					char buff[256];
					string sNodeID;
					sprintf(buff, "%i", rechargeList[i].nodeId);
					sNodeID.assign(buff);
					string sNextHopId = getControllerId(rechargeList[i].location);

					FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
					advMsg->setByteLength(netDataFrameOverhead);
					advMsg->setFlyForeverRoutingPacketKind(FlyForever_REPO_MSG);
					advMsg->setSource(SELF_NETWORK_ADDRESS);
					advMsg->setDestination(sNodeID.c_str());
					if (atoi(sNextHopId.c_str()) == self)
						advMsg->setNextHop(sNodeID.c_str());
					else
						advMsg->setNextHop(sNextHopId.c_str());
					advMsg->setIdealLocation(getGCSlocation());
					advMsg->setRecharging(true);
					toMacLayer(advMsg, resolveNetworkAddress(advMsg->getNextHop()));
					if (atoi(sNextHopId.c_str()) == self)
						trace() << "[TCP] C_" << self << " is sending a energy replacement msg to node_" << advMsg->getDestination() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")";
					else
						trace() << "[TCP] C_" << self << " is sending a energy replacement msg to node_" << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")";
				}
			}
			/*for (int i=0; i<replaceList.size(); i++){
				if (replaceList[i].ack == false){
					needAck=true;
					int val = -1;
					for (int j=0; j<nodes.size(); j++){
						if (nodes[j].id == replaceList[i].nodeId){
							val = j;
							break;
						}
					}
					char buff[256];
					string sNodeID;
					sprintf(buff, "%i", replaceList[i].nodeId);
					sNodeID.assign(buff);
					string sNextId;
					sprintf(buff, "%i", replaceList[i].nextHop);
					sNextId.assign(buff);
					string sNextHopId = getControllerId(nodes[val].selfLocation);

					FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
					advMsg->setByteLength(netDataFrameOverhead);
					advMsg->setFlyForeverRoutingPacketKind(FlyForever_RN_MSG);
					advMsg->setSource(SELF_NETWORK_ADDRESS);
					advMsg->setDestination(sNodeID.c_str());
					if (atoi(sNextHopId.c_str()) == self)
						advMsg->setNextHop(sNodeID.c_str());
					else
						advMsg->setNextHop(sNextHopId.c_str());
					advMsg->setNextHopRoute(sNextId.c_str());
					advMsg->setIdealLocation(replaceList[i].location);
					advMsg->setSourceRoute(replaceList[i].source);
					advMsg->setReplacement(true);
					toMacLayer(advMsg, resolveNetworkAddress(advMsg->getDestination()));

					if (atoi(sNextHopId.c_str()) == self)
						trace() << "C_" << self << " is sending a replacement msg to node " << advMsg->getDestination() << " with next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")\n";
					else
						trace() << "C_" << self << " is sending a replacement msg to node " << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")\n";
					
				}
			}*/
			trace() << "rechargeList.size() " << rechargeList.size();
			if (SIMTIME_DBL(simTime()) > sim_time_limit - 5){
				rechargeList.clear();
				route.clear();
			}
			trace() << "rechargeList.size() " << rechargeList.size();
			trace() << "route.size() " << route.size();
			if (needAck)
				setTimer(FlyForever_CHECKACK, 0.1);
			else
		break;
		}
	}
}

//================================================================
//    sendAdv
//================================================================
void FlyForeverRouting::sendAdv(){
	int index = getControllerIndex(getCurrentLocation());
	string sNodeID = getControllerId(getCurrentLocation());
	if(computeDistance(getCurrentLocation(),controllerList[index].controllerLocation) <= 60)
		setRadio_TXPowerLevel(12,0);
	else
		setRadio_TXPowerLevel(30,0);

	FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
	advMsg->setByteLength(netDataFrameOverhead);
	advMsg->setFlyForeverRoutingPacketKind(FlyForever_ADV_MSG);
	advMsg->setSource(SELF_NETWORK_ADDRESS);
	advMsg->setDestination(controllerAddress.c_str());
	advMsg->setNextHop(sNodeID.c_str());
	advMsg->setRE(getResidualEnergy());
	advMsg->setCurrentLocation(getCurrentLocation());
	advMsg->setSpeed(mobilityModule->getSpeed());
	advMsg->setData(sensedValue);
	advMsg->setTraject(mobilityModule->getMovingTrajectory());
	toMacLayer(advMsg, resolveNetworkAddress(advMsg->getNextHop()));
	setRadio_TXPowerLevel(12,0.05);
	//trace() << "distance to controler " << computeDistance(controllerList[index].controllerLocation, getCurrentLocation());
	//trace() << "Node_" << self << " is sending an adv msg to the controller_" << advMsg->getDestination() << " via controller_" << advMsg->getNextHop() << " with RE: " << advMsg->getRE() << " located at " << advMsg->getCurrentLocation().x << "," << advMsg->getCurrentLocation().y << " moving at " << advMsg->getSpeed() << "m/s";
}

//================================================================
//    selectSource
//================================================================
void FlyForeverRouting::selectSource(){
        int index;
	stringstream ss;
	trace() << "event: (" << eventList[idEvent].eventLocation.x << ", " << eventList[idEvent].eventLocation.y << ")";
	trace() << "DN: (" << destinationLocation.x << ", " << destinationLocation.y << ")";
	for (int i=0; i<nodes.size(); i++){
		double distance = computeDistance(eventList[idEvent].eventLocation, nodes[i].selfLocation);
		double timeToMove;
		if (mobilityModule->getMaxSpeed() == 5)
			timeToMove = distance / (mobilityModule->getMaxSpeed() * 2.5);
		else
			timeToMove = distance / mobilityModule->getMaxSpeed();

		double Efly = distance * ((0.95 * (mobilityModule->getMaxSpeed() * mobilityModule->getMaxSpeed())) - (20.4 * mobilityModule->getMaxSpeed()) +130);

		double energyPerSample = 0.5;
		double Evideo = 300 * energyPerSample;

        	double distanceToGCS = computeDistance(nodes[i].selfLocation, getGCSlocation());
        	double EtoGCS = distanceToGCS * ((0.95 * (mobilityModule->getMinSpeed() * mobilityModule->getMinSpeed())) - (20.4 * mobilityModule->getMinSpeed()) +130);

		double Eest = Evideo + Efly + EtoGCS;
		if (timeToMove<timeToMoveMax && Eest < getInitialEnergy()){
			nodes[i].avg = 0.3*(nodes[i].RE/getInitialEnergy())+0.7*(timeToMove/timeToMoveMax);
			nodes[i].timeMove = timeToMove;
		} else{
			nodes[i].avg = 1;
			nodes[i].timeMove = timeToMove;
		}
		if((int) nodes[i].time < (int) SIMTIME_DBL(simTime()) - 10)
			nodes[i].avg = 2;
		
	}

	sort(nodes.begin(), nodes.end(), FlyForever_sort_node);

	bool sorted = false;
	if (nodes.size() > 0){
		if (nodes[0].avg == 1){
			sorted = true;
			sort(nodes.begin(), nodes.end(), FlyForever_sort_time);
		}
	}

	trace()<<"time\tnodeId\tEt\tLoc.x\tLoc.y\tTraj.\tSpeed\ttMove\troute\tavg\tdist";
	for (int i=0; i<4; i++)
		trace() << (int) nodes[i].time << "\t" << nodes[i].id << "\t" << nodes[i].RE << "\t" << nodes[i].selfLocation.x << "\t" << nodes[i].selfLocation.y << "\t" << nodes[i].trajectory << "\t" << nodes[i].speed << "\t" << nodes[i].timeMove << "\t" << nodes[i].selected << "\t" << nodes[i].avg << "\t" << computeDistance(eventList[idEvent].eventLocation, nodes[i].selfLocation);

	if (nodes.size() > 0){
		routeCInfo temp;
		temp.source = nodes[0].id;
		temp.destination = atoi(destinationAddress.c_str());
		temp.nodeId = nodes[0].id;
		temp.idealLocation = eventList[idEvent].eventLocation;
		temp.selfLocation = nodes[0].selfLocation;
		temp.ack = false;
		temp.replace = false;
		route.push_back(temp);
		nodes[0].selected = true;
		tMax = nodes[0].timeMove;
		tStart = nodes[0].timeMove;
		trace() << "Selected SN, Node id " << temp.nodeId;
		trace() << "SN location: (" << temp.idealLocation.x << ", " << temp.idealLocation.y << ")";
		trace() << "distance to ideal location " << computeDistance(eventList[idEvent].eventLocation, nodes[0].selfLocation);
		trace() << "distance from ideal location to DN " << computeDistance(eventList[idEvent].eventLocation, destinationLocation);
		trace() << "distance from RN to C_20 " << computeDistance(nodes[0].selfLocation, controllerList[0].controllerLocation);
		trace() << "tMax " << tMax;
		trace() << "tStart " << tStart;
		trace() << "timeToMove " << nodes[0].timeMove << "\n";
		setTimer(FlyForever_DECISIONRN, 0);
	}
}

//================================================================
//    selectRelay
//================================================================
void FlyForeverRouting::selectRelay(){
	double RR = 55;
	int index;
	FlyForeverLocationInfo idealLocation = computeIdealLocation();

    	for (int i=0; i<nodes.size(); i++){
		//TODO: Média para verificar o melhor node para ser o nó fonte.
		double distance = computeDistance(idealLocation, nodes[i].selfLocation);
		double timeToMove;
		if (mobilityModule->getMaxSpeed() == 5)
			timeToMove = distance / (mobilityModule->getMaxSpeed() * 2.5);
		else
			timeToMove = distance / mobilityModule->getMaxSpeed();

		double Efly = (mobilityModule->getEnergyToFly()/mobilityModule->getMaxSpeed()) * distance;

		double Eforward = 20;

        	double distanceToGCS = computeDistance(nodes[i].selfLocation, getGCSlocation());
        	double EtoGCS = distanceToGCS * ((0.95 * (mobilityModule->getMinSpeed() * mobilityModule->getMinSpeed())) - (20.4 * mobilityModule->getMinSpeed()) + 130);

		double Eest = Eforward + Efly + EtoGCS;

		if (timeToMove<tMax && Eest < getInitialEnergy()){
			nodes[i].avg=0.3*(nodes[i].RE/getInitialEnergy())+0.7*(timeToMove/timeToMoveMax);
			nodes[i].timeMove = timeToMove;
		}else{
			nodes[i].avg = 1;
			nodes[i].timeMove = timeToMove;
		}
		if((int) nodes[i].time < (int) SIMTIME_DBL(simTime()) - 10){
			nodes[i].avg = 2;
			nodes[i].selected = true;
			nodes[i].replacement = true;
		}
	}

	sort(nodes.begin(), nodes.end(), FlyForever_sort_node);

	for (int i=0; i<nodes.size(); i++){
		if (nodes[i].selected == false){
			index = i;
			break;
		}
	}

	bool sorted = false;
	if (nodes.size() > 0){
		if (nodes[index].avg == 1){
			sorted = true;
			sort(nodes.begin(), nodes.end(), FlyForever_sort_time);
		}
	}

	if (sorted){
		for (int i=0; i<nodes.size(); i++){
			if (nodes[i].selected == false){
				index = i;
				break;
			}
		}
	}

	trace()<<"time\tnodeId\tEt\tLoc.x\tLoc.y\tTraj.\tSpeed\ttMove\troute\tavg\tTime\trepla";
	for (int i=0; i<4; i++)
		trace() << (int) nodes[i].time << "\t" << nodes[i].id << "\t" << nodes[i].RE << "\t" << nodes[i].selfLocation.x << "\t" << nodes[i].selfLocation.y << "\t" << nodes[i].trajectory << "\t" << nodes[i].speed << "\t" << nodes[i].timeMove << "\t" << nodes[i].selected << "\t" << nodes[i].avg  << "\t" << nodes[i].time << "\t" << nodes[i].replacement;

	if (nodes.size() > 0){
		route[route.size()-1].nextHop = nodes[index].id;
		routeCInfo temp;
		temp.source = route[0].nodeId;
		temp.destination = atoi(destinationAddress.c_str());
		temp.nodeId = nodes[index].id;
		temp.idealLocation = idealLocation;
		temp.selfLocation = nodes[index].selfLocation;
		temp.ack = false;
		temp.replace = false;
		temp.nextHop = -1;
		route.push_back(temp);
		nodes[index].selected = true;

		if (nodes[index].timeMove > tStart)
			tStart = nodes[index].timeMove;

		trace() << "Selected RN, node id " << temp.nodeId;
		trace() << "distance to ideal location " << computeDistance(idealLocation, nodes[index].selfLocation);
		trace() << "distance from ideal location to DN " << computeDistance(idealLocation, destinationLocation);
		trace() << "distance from RN to C_20 " << computeDistance(nodes[index].selfLocation, controllerList[0].controllerLocation);
		trace() << "tMax " << tMax;
		trace() << "tStart " << tStart;
		trace() << "time to move " << nodes[index].timeMove << "\n";
		double distSelfDN = computeDistance(idealLocation, destinationLocation);
		if (distSelfDN > RR){
			setTimer(FlyForever_DECISIONRN, 0);
		} else{
			route[route.size()-1].nextHop=atoi(destinationAddress.c_str());
			setTimer(FlyForever_SENDINFO, 0);
		}
	}
}

//================================================================
//    nodeRecharging
//================================================================
void FlyForeverRouting::nodeRecharging(int index, int src, FlyForeverLocationInfo loc){
		trace() << "[nodeRecharging]";
		trace() << "Recharge R_" << src;

		char buff[256];
		string sNodeID;
		sprintf(buff, "%i", src);
		sNodeID.assign(buff);
		string sNextHopId = getControllerId(loc);

		FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
		advMsg->setByteLength(netDataFrameOverhead);
		advMsg->setFlyForeverRoutingPacketKind(FlyForever_REPO_MSG);
		advMsg->setSource(SELF_NETWORK_ADDRESS);
		advMsg->setDestination(sNodeID.c_str());
		if (atoi(sNextHopId.c_str()) == self)
			advMsg->setNextHop(sNodeID.c_str());
		else
			advMsg->setNextHop(sNextHopId.c_str());
		advMsg->setIdealLocation(getGCSlocation());
		advMsg->setRecharging(true);
		toMacLayer(advMsg, resolveNetworkAddress(advMsg->getDestination()));
		if (atoi(sNextHopId.c_str()) == self)
			trace() << "C_" << self << " is sending a node Recharging msg to node_" << advMsg->getDestination() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")";
		else
			trace() << "C_" << self << " is sending a node Recharging msg to node_" << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y;
}

//================================================================
//    nodeReplacement
//================================================================
void FlyForeverRouting::nodeReplacement(int index, int src){
	trace() << "replace F_" << src;
	route[index].replace = true;
	trace()<<"src\tdst\tId\tnext\tLoc.x\tLoc.y\tack\treplace";
	trace() << route[index].source << "\t" << route[index].destination << "\t" << route[index].nodeId << "\t" << route[index].nextHop << "\t" << route[index].idealLocation.x << "\t" << route[index].idealLocation.y << "\t" << route[index].ack << "\t" << route[index].replace;

	idealLocation = route[index].idealLocation;
	for (int i=0; i<nodes.size(); i++){
		double distance = computeDistance(idealLocation, nodes[i].selfLocation);
		double timeToMove;
		if (mobilityModule->getMaxSpeed() == 5)
			timeToMove = distance / (mobilityModule->getMaxSpeed() * 2);
		else
			timeToMove = distance / mobilityModule->getMaxSpeed();

		double Efly = (mobilityModule->getEnergyToFly()/mobilityModule->getMaxSpeed()) * distance;

		double Eforward = 20;

		double distanceToGCS = computeDistance(nodes[i].selfLocation, getGCSlocation());
        	double EtoGCS = distanceToGCS * ((0.95 * (mobilityModule->getMinSpeed() * mobilityModule->getMinSpeed())) - (20.4 * mobilityModule->getMinSpeed()) + 130);

		double Eest = Eforward + Efly + EtoGCS;

		if (timeToMove<timeToMoveMax && Eest < getInitialEnergy()){
			nodes[i].avg = 0.3*(nodes[i].RE/getInitialEnergy())+0.7*(timeToMove/timeToMoveMax);
			nodes[i].timeMove = timeToMove;
		}else{
			nodes[i].avg = 1;
			nodes[i].timeMove = timeToMove;
		}
	}

	sort(nodes.begin(), nodes.end(), FlyForever_sort_node);
	trace()<<"time\tnodeId\tEt\tLoc.x\tLoc.y\tTraj.\tSpeed\ttMove\troute\tavg";
	for (int i=0; i<nodes.size()/3; i++)
		trace() << (int) nodes[i].time << "\t" << nodes[i].id << "\t" << nodes[i].RE << "\t" << nodes[i].selfLocation.x << "\t" << nodes[i].selfLocation.y << "\t" << nodes[i].trajectory << "\t" << nodes[i].speed << "\t" << nodes[i].timeMove << "\t" << nodes[i].selected << "\t" << nodes[i].avg;

	int val = -1;
	for (int i=0; i<nodes.size(); i++){
		if (nodes[i].selected == false){
			val = i;
			break;
		}
	}

	bool sorted = false;
	if (nodes.size() > 0){
		if (nodes[val].avg == 1){
			sorted = true;
			sort(nodes.begin(), nodes.end(), FlyForever_sort_time);
		}
	}

	if (sorted){
		for (int i=0; i<nodes.size(); i++){
			if (nodes[i].selected == false){
				val = i;
				break;
			}
		}
	}

	if (nodes.size() > 0){
		replacementInfo temp;
		temp.source = route[0].nodeId;
		temp.destination = atoi(destinationAddress.c_str());
		temp.nodeId = nodes[val].id;
		temp.location = idealLocation;
		temp.line = index;
		temp.ack = false;
		temp.replace = false;
		temp.nextHop = route[index].nextHop;
		replaceList.push_back(temp);
		nodes[val].selected = true;

		trace() << "src id " << temp.source;
		trace() << "dst id " << temp.destination;
		trace() << "node id " << temp.nodeId;
		trace() << "next-hop id " << temp.nextHop;
		trace() << "route id " << temp.line;
		trace() << "distance to ideal location " << computeDistance(idealLocation, nodes[index].selfLocation);
		trace() << "distance from ideal location to DN " << computeDistance(idealLocation, destinationLocation);
		trace() << "time to move " << nodes[index].timeMove;

		char buff[256];
		string sNodeID;
		sprintf(buff, "%i", temp.nodeId);
		sNodeID.assign(buff);
		string sNextId;
		sprintf(buff, "%i", temp.nextHop);
		sNextId.assign(buff);
		string sNextHopId = getControllerId(nodes[val].selfLocation);


		FlyForeverRoutingPacket *advMsg = new FlyForeverRoutingPacket("Node is sending an adv msg", NETWORK_LAYER_PACKET);
		advMsg->setByteLength(netDataFrameOverhead);
		advMsg->setFlyForeverRoutingPacketKind(FlyForever_RN_MSG);
		advMsg->setSource(SELF_NETWORK_ADDRESS);
		advMsg->setDestination(sNodeID.c_str());
		if (atoi(sNextHopId.c_str()) == self)
			advMsg->setNextHop(sNodeID.c_str());
		else
			advMsg->setNextHop(sNextHopId.c_str());
		advMsg->setNextHopRoute(sNextId.c_str());
		advMsg->setIdealLocation(temp.location);
		advMsg->setSourceRoute(temp.source);
		advMsg->setReplacement(true);
		toMacLayer(advMsg, resolveNetworkAddress(advMsg->getDestination()));

		if (atoi(sNextHopId.c_str()) == self)
			trace() << "C_" << self << " is sending a node replacement msg to node " << advMsg->getDestination() << " with next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")\n";
		else
			trace() << "C_" << self << " is sending a node replacement msg to node " << advMsg->getDestination() << " via C_" << advMsg->getNextHop() << " -> ROUTE: next-hop " << advMsg->getNextHopRoute() << " to move to (" << advMsg->getIdealLocation().x << ", " << advMsg->getIdealLocation().y << ")\n";
	}
}

//================================================================
//    dropAlgorithm
//================================================================
void FlyForeverRouting::dropAlgorithm(FlyForeverRoutingPacket *dupPacket){
	trace() << "F_" << self << " has to drop packet number " << buffer[0].seqNumber << " using drop tail algorithm!";
	trace() << "Before - Routing buffer state: " << buffer.size() << "/" << netBufferSize;
	buffer.erase(buffer.begin()+0);
	//statistics(atoi(dupPacket->getSource()), atoi(dupPacket->getDestination()), dupPacket->getIdVideo(), dupPacket->getHopCount(), 0, FlyForever_STATISTICS_DROP_NTW);
	trace() << "After - Routing buffer state: " << buffer.size() << "/" << netBufferSize;
}

//================================================================
//    SNbackboneMode
//================================================================
void FlyForeverRouting::SNbackboneMode(cPacket *pkt, int idVideo){
	int index = -1;
	for (int i = 0; i<route.size(); i++){
		if (route[i].source == self && route[i].destination == atoi(destinationAddress.c_str())){
			index = i;
			break;
		}
	}
	if (index != -1){
		char buff[256];
		string sNextId;
		sprintf(buff, "%i", route[index].nextHop);
		sNextId.assign(buff);

		FlyForeverRoutingPacket *multiPkt = new FlyForeverRoutingPacket("SN is sending multimedia pkt", NETWORK_LAYER_PACKET);
		multiPkt->setFlyForeverRoutingPacketKind(FlyForever_MULTIMEDIA_PKT);
		multiPkt->setSource(SELF_NETWORK_ADDRESS);
		multiPkt->setDestination(destinationAddress.c_str());
		multiPkt->setNextHop(sNextId.c_str());
		multiPkt->setHopCount(0);
		multiPkt->setIdVideo(idVideo);
		encapsulatePacket(multiPkt, pkt);
		toMacLayer(multiPkt, resolveNetworkAddress(sNextId.c_str()));
		statisticLastPkt = multiPkt->getSequenceNumber(); //TODO: tempo de rota
		statisticQtdPkts ++; //TODO: tempo de rota
		print = false; //TODO: tempo de rota
		trace() << "FlyForever_BACKBONE_MODE - SN_" << self << " is sending the multimedia pkt number " << multiPkt->getSequenceNumber() << " to DN_" << multiPkt->getDestination() << " to F_" << multiPkt->getNextHop() << "\n";
		if (getTimer(FlyForever_ROUTE_STATISTICS) == -1){
			setTimer(FlyForever_ROUTE_STATISTICS, 12);
			//TODO: tempo de rota
			statisticNext = atoi(multiPkt->getNextHop());
			statisticFistPkt = multiPkt->getSequenceNumber();
			statisticLastPkt = multiPkt->getSequenceNumber();
			statisticDuration = SIMTIME_DBL(simTime());
			statisticIdVideo = multiPkt->getIdVideo();
			statisticIdSource = atoi(multiPkt->getSource());
			statisticQtdPkts = 1;
			print = false;
			//trace() << statisticIdVideo << "\t" << statisticIdSource << "\t" << self << "\t" << statisticNext << "\t" << statisticFistPkt << "\t" << statisticLastPkt << "\t" << statisticQtdPkts << "\t" << statisticDuration;
			//TODO: tempo de rota
		}
	}
}

//================================================================
//    RNreceivedPktBackbone
//================================================================
void FlyForeverRouting::RNreceivedPktBackbone(FlyForeverRoutingPacket *netPacket){
	trace() << "XLINGOTC_BACKBONE_MODE - F_" << self << " received a multimedia pkt number " << netPacket->getSequenceNumber() << " from SN_" << netPacket->getSource() << " to DN_" << netPacket->getDestination() << " routed by F_" << netPacket->getSource() << " with " << netPacket->getHopCount() + 1 << " hops";
	trace() << "SN_" << self << " location (" << getCurrentLocation().x << "," << getCurrentLocation().y << ")" << " => Ideal Location (" << route[0].idealLocation.x << "," << route[0].idealLocation.y << ")";
	trace() << "Distance to ideal " << computeDistance(route[0].idealLocation, getCurrentLocation());
	trace() << "Distance ideal to DN " << computeDistance(route[0].idealLocation, destinationLocation);
	trace() << "distance current to DN " << computeDistance(getCurrentLocation(),destinationLocation);
	int index = -1;
	for (int i = 0; i<route.size(); i++){
		if (route[i].source == atoi(netPacket->getSource()) && route[i].destination == atoi(destinationAddress.c_str())){
			index = i;
			break;
		}
	}
	if (index != -1){
		char buff[256];
		string sNextId;
		sprintf(buff, "%i", route[index].nextHop);
		sNextId.assign(buff);
		FlyForeverRoutingPacket *dupPacket = netPacket->dup();
		dupPacket->setHopCount(netPacket->getHopCount() +1);
		dupPacket->setNextHop(sNextId.c_str());
		toMacLayer(dupPacket, resolveNetworkAddress(sNextId.c_str()));
		trace() << "F_" << self << " is forwarding the multimedia pkt number " << netPacket->getSequenceNumber() << " to F_" << route[index].nextHop << "\n";

		statistics(atoi(dupPacket->getSource()), atoi(dupPacket->getDestination()), dupPacket->getIdVideo(), dupPacket->getHopCount(), 0, FlyForever_STATISTICS_UNICAST_PACKET);
		statisticLastPkt = dupPacket->getSequenceNumber(); //TODO: tempo de rota
		statisticQtdPkts ++; //TODO: tempo de rota
		print = false; //TODO: tempo de rota
		if (getTimer(FlyForever_ROUTE_STATISTICS) == -1){
			setTimer(FlyForever_ROUTE_STATISTICS, 12);
			//TODO: tempo de rota
			statisticNext = atoi(dupPacket->getNextHop());
			statisticFistPkt = dupPacket->getSequenceNumber();
			statisticLastPkt = dupPacket->getSequenceNumber();
			statisticDuration = SIMTIME_DBL(simTime());
			statisticIdVideo = dupPacket->getIdVideo();
			statisticIdSource = atoi(dupPacket->getSource());
			statisticQtdPkts = 1;
			print = false;
			trace() << statisticIdVideo << "\t" << statisticIdSource << "\t" << self << "\t" << statisticNext << "\t" << statisticFistPkt << "\t" << statisticLastPkt << "\t" << statisticQtdPkts << "\t" << statisticDuration;
			//TODO: tempo de rota
		}
	}
}

//================================================================
//    DNreceivedPkt
//================================================================
void FlyForeverRouting::DNreceivedPkt(cPacket *pkt){
	FlyForeverRoutingPacket *netPacket = dynamic_cast <FlyForeverRoutingPacket*>(pkt);
	trace() << "DN received multimedia pkt number " << netPacket->getSequenceNumber() << " from SN_" << netPacket->getSource() << " to DN_" << netPacket->getDestination() << "routed by F_" << netPacket->getNextHop() << " with " << netPacket->getHopCount() + 1 << " hops"; 

	bool notDuplicate = isNotDuplicatePacket(pkt);
	if (notDuplicate){
		FlyForeverRoutingPacket *packet = dynamic_cast <FlyForeverRoutingPacket*>(pkt);
		FlyForeverRoutingPacket *netPkt = packet->dup();
		ApplicationPacket *appPkt = check_and_cast <ApplicationPacket*>(decapsulatePacket(netPkt));
		int index = -1;
		for (int i = 0; i<route.size(); i++){
			if (route[i].source == atoi(netPacket->getSource()) && route[i].destination == atoi(destinationAddress.c_str())){
				index = i;
				break;
			}
		}
		if (index == -1){
			routeCInfo table;
			table.source = atoi(netPacket->getSource());
			table.destination = atoi(netPacket->getDestination());
			table.lastReceivedPacket = -1;
			route.push_back(table);
			index = route.size() - 1;
		}

		double delay = (simTime().dbl() - appPkt->getAppNetInfoExchange().timestamp.dbl())*1000;
		if (index != -1){
			if((netPacket->getSequenceNumber() == 0 && route[index].lastReceivedPacket == -1) || netPacket->getSequenceNumber() == route[index].lastReceivedPacket + 1 || route[index].lastReceivedPacket == -1){
				if (delay > macModule->getDelayMax()){ //drop due to packet deadline
					trace() << "Drop packet - Delay (" << delay << ")higher than the playout deadline(" << macModule->getDelayMax() << ")";
					route[index].lastReceivedPacket = netPacket->getSequenceNumber();
					//statistics(atoi(netPacket->getSource()), atoi(netPacket->getDestinationLinGO()), netPacket->getIdVideo(), netPacket->getHopCount() + 1, 0,  XLINGOTC_STATISTICS_DROP_DELAY);
				} else { //send packet to application layer
					trace() << "receive a subsequent pkt number " << netPacket->getSequenceNumber() << " and send it to application";
					toApplicationLayer(decapsulatePacket(pkt));
					route[index].lastReceivedPacket = netPacket->getSequenceNumber();
					statistics(atoi(netPacket->getSource()), atoi(netPacket->getDestination()), netPacket->getIdVideo(), netPacket->getHopCount() + 1, 0,  FlyForever_STATISTICS_RECEIVED_PACKET);
					if (buffer.size() > 0 && searchBuffer(atoi(netPacket->getSource()))){
						sort(buffer.begin(), buffer.end(), FlyForever_sort_buffer);
						bool tmp = true;
						while(buffer.size() > 0 && searchBuffer(atoi(netPacket->getSource())) && tmp){
							tmp = false;
							for (int i=0; i<buffer.size(); i++){
								FlyForeverRoutingPacket *packet = dynamic_cast <FlyForeverRoutingPacket*>(buffer[i].bufferPkt.front());
								if (atoi(packet->getSource()) == atoi(netPacket->getSource()) && buffer[i].seqNumber == route[index].lastReceivedPacket + 1){
									trace() << "Send pkt number " << buffer[i].seqNumber << " to application";
									toApplicationLayer(decapsulatePacket(buffer[i].bufferPkt.front()));
									route[index].lastReceivedPacket = buffer[i].seqNumber;
									buffer.erase(buffer.begin()+i);
									tmp = true;
									break;
								}
							}
						}
					}
				}
			}else {
				if (route[index].lastReceivedPacket < netPacket->getSequenceNumber()){
					if ( delay > macModule->getDelayMax()){ //drop due to packet deadline
						trace() << "Drop packet - Delay (" << delay << ")higher than the playout deadline(" << macModule->getDelayMax() << ")";
						//statistics(atoi(netPacket->getSource()), atoi(netPacket->getDestinationLinGO()), netPacket->getIdVideo(), netPacket->getHopCount() + 1, 0,  XLINGOTC_STATISTICS_DROP_DELAY);
					} else { //buffer the packet
						pktList temp;
						temp.seqNumber = netPacket->getSequenceNumber();
						temp.bufferPkt.push(netPacket->dup());
						buffer.push_back(temp);
						statistics(atoi(netPacket->getSource()), atoi(netPacket->getDestination()), netPacket->getIdVideo(), netPacket->getHopCount() + 1, 0,  FlyForever_STATISTICS_RECEIVED_PACKET);
						setMultipleTimer(FlyForever_CLEAN_BUFFER, atoi(netPacket->getSource()), 1);
						trace() << "Buffered pkt number " << netPacket->getSequenceNumber() << " buffer state: " << buffer.size() << "/" << netBufferSize;
					}
				}  else{
					trace() << "Drop packet number " << netPacket->getSequenceNumber() << ", due to it is old. Last Packet sent to App Layer";
					trace() << "last rec pkt " << route[index].lastReceivedPacket << " for source " << route[index].source;
					//statistics(atoi(netPacket->getSource()), atoi(netPacket->getDestinationLinGO()), netPacket->getIdVideo(), 0, 0, XLINGOTC_STATISTICS_DROP_NTW);
				}
				if (buffer.size() == netBufferSize){ //send bufferede packets to application layer
					sort(buffer.begin(), buffer.end(), FlyForever_sort_buffer);
					while(buffer.size() > (netBufferSize/2.0)){
						FlyForeverRoutingPacket *packet = dynamic_cast <FlyForeverRoutingPacket*>(buffer[0].bufferPkt.front());
						trace() << "Send pkt number " << buffer[0].seqNumber << " to application from source " << packet->getSource();
						//int tmpIndex = getNextHopIndex(packet->getSource(), netPacket->getDestination());
						int tmpIndex = index;
						toApplicationLayer(decapsulatePacket(buffer[0].bufferPkt.front()));
						route[tmpIndex].lastReceivedPacket = buffer[0].seqNumber;
						buffer.erase(buffer.begin()+0);
					}
					bool tmp = true;
					while(buffer.size() > 0 && searchBuffer(atoi(netPacket->getSource())) && tmp){
						tmp = false;
						for (int i=0; i<buffer.size(); i++){
							FlyForeverRoutingPacket *packet = dynamic_cast <FlyForeverRoutingPacket*>(buffer[i].bufferPkt.front());
							if (atoi(packet->getSource()) == atoi(netPacket->getSource()) && buffer[i].seqNumber == route[index].lastReceivedPacket + 1){
								trace() << "Send pkt number " << buffer[i].seqNumber << " to application from source " << packet->getSource();
								toApplicationLayer(decapsulatePacket(buffer[i].bufferPkt.front()));
								//int tmpIndex = getNextHopIndex(packet->getSource(), netPacket->getDestination());
								int tmpIndex = index;
								route[tmpIndex].lastReceivedPacket = buffer[i].seqNumber;
								buffer.erase(buffer.begin()+i);
								tmp = true;
								break;
							}
						}
					}
					if (buffer.size() == 0)
						cancelMultipleTimer(FlyForever_CLEAN_BUFFER, atoi(netPacket->getSource()));
				}
			}
		}

	} else if (!notDuplicate){
		///statistics(atoi(netPacket->getSource()), atoi(netPacket->getDestinationLinGO()), netPacket->getIdVideo(), netPacket->getHopCount() + 1, 0,  XLINGOTC_STATISTICS_DUPLICATED_PACKET);
		trace() << "DN received a duplicated packet";
	}
	trace() << "Routing buffer state: " << buffer.size() << "/" << netBufferSize;
	trace() << "MAC control buffer state: " << macModule->getControlBufferSize() << "/" << macModule->macBufferSize;
	trace() << "MAC buffer state: " << macModule->getBufferSize() << "/" << macModule->macBufferSize << "\n";
}

//================================================================
//    getControllerIndex
//================================================================
int FlyForeverRouting::getControllerIndex(FlyForeverLocationInfo nodeLocation){
	double dMin = 200;
	int index = 0;
	if(computeDistance(nodeLocation, controllerList[index].controllerLocation) > 90){
		for(int j=1; j<controllerList.size(); j++){
			double distance = computeDistance(nodeLocation, controllerList[j].controllerLocation);
			if (distance < dMin){
				dMin = distance;
				index = j;
			}
		}
	}
	return index;
}

//================================================================
//    getControllerId
//================================================================
string FlyForeverRouting::getControllerId(FlyForeverLocationInfo loc){
	double dMin = 200;
	int index = 0;
	if(computeDistance(loc, controllerList[index].controllerLocation) > 90){
		for(int j=1; j<controllerList.size(); j++){
			double distance = computeDistance(loc, controllerList[j].controllerLocation);
			//trace() << "node_" << self << " distance " << distance << " controler id_ " << controllerList[j].controllerId;
			if (distance < dMin){
				dMin = distance;
				index = j;
			}
		}
	}
	char buff[256];
	string sNextHopId;
	sprintf(buff, "%i", controllerList[index].controllerId);
	sNextHopId.assign(buff);
	return sNextHopId.c_str();
}

//================================================================
//    searchBuffer
//================================================================
bool FlyForeverRouting::searchBuffer(int source){
	for (int i=0; i<buffer.size(); i++){
		FlyForeverRoutingPacket *packet = dynamic_cast <FlyForeverRoutingPacket*>(buffer[i].bufferPkt.front());
		if(atoi(packet->getSource()) == source){
			return true;
		}
	}
	return false;
}

//================================================================
//    processBufferedPacket
//================================================================
void FlyForeverRouting::processBufferedPacket(string source) {
	if (buffer.size() > 0) {
		for (int i=0; i<buffer.size(); i++){

		}
	}
}

//================================================================
//    computeIdealLocation
//================================================================
FlyForeverLocationInfo FlyForeverRouting::computeIdealLocation(){
	FlyForeverLocationInfo idealLocation;
	double RR = 55;
	double dist = mobilityModule->getMovimentRange();
	//calculo do idealLocation com as informações do Source 
	if(route.size()>0){
		idealLastHopLocation = route[route.size()-1].idealLocation;

	}
	double distRNDN = computeDistance(idealLastHopLocation, destinationLocation);
	double coordenatex = ((RR-(2*dist))*(idealLastHopLocation.x - destinationLocation.x))/distRNDN;
	double coordenatey = ((RR-(2*dist))*(idealLastHopLocation.y - destinationLocation.y))/distRNDN;
	idealLocation.x = idealLastHopLocation.x - coordenatex;
	if (idealLocation.x < 0)
		idealLocation.x = 0;
	idealLocation.y = idealLastHopLocation.y - coordenatey;
	if (idealLocation.y < 0)
		idealLocation.y = 0;
	trace() << "last location: (" << idealLastHopLocation.x << ", " << idealLastHopLocation.y << ")";
	trace() << "ideal location: (" << idealLocation.x << ", " << idealLocation.y << ")";
	return idealLocation;
}

//================================================================
//    computeDistance
//================================================================
double FlyForeverRouting::computeDistance(FlyForeverLocationInfo source, FlyForeverLocationInfo dest){
	double distance, dx, dy;
	dx = dest.x - source.x;
	dy = dest.y - source.y;
	distance = sqrt((dx * dx) + (dy * dy));
	return distance;
}

//================================================================
//    getCurrentLocation
//================================================================
FlyForeverLocationInfo FlyForeverRouting::getCurrentLocation(){
	location = mobilityModule->getLocation();
	currentLocation.x = location.x;
	currentLocation.y = location.y;
	return currentLocation;
}

//================================================================
//    getInitialEnergy
//================================================================
double FlyForeverRouting::getInitialEnergy(){
	return srceRerouceModuce->getInitialEnergy();
}

//================================================================
//    getResidualEnergy
//================================================================
double FlyForeverRouting::getResidualEnergy(){
	return resMgrModule->getRemainingEnergy();
}

//================================================================
//    getRadioRange
//================================================================
double FlyForeverRouting::getRadioRange(){
	double sensitivity = getSensitivity();
	double noiseFloor = radioModule->getNoiseFloor();
	double d0 = wirelessModule->getD0();
	double PLd0 = wirelessModule->getPLd0();
	double pathLossExponent = wirelessModule->getPathLossExponent();
	double RR = d0 * pow(10.0, (getTxPower() - max(sensitivity, noiseFloor+5) - PLd0) / (10.0 * pathLossExponent));
	if (getTxPower() == 12)
		return 55;
	else
		return RR;
}

//================================================================
//    getSensitivity
//================================================================
double FlyForeverRouting::getSensitivity(){
	return radioModule->CCAthreshold;
}

//================================================================
//    getTxPower
//================================================================
double FlyForeverRouting::getTxPower(){
	return radioModule->TxLevel->txOutputPower;
}

//================================================================
//    getEnergyPerPkt
//================================================================
double FlyForeverRouting::getEnergyPerPkt(){
	return radioModule->TxLevel->txPowerConsumed / 1000.0;
}

//================================================================
//    setRadio_Listen
//================================================================
void FlyForeverRouting::setRadio_Listen(double delay){
	if(delay == 0.0)
		send(createRadioCommand(SET_STATE, RX), "toMacModule");	
	else
		sendDelayed(createRadioCommand(SET_STATE, RX), delay, "toMacModule");
}

//================================================================
//    setRadio_Sleep
//================================================================
void FlyForeverRouting::setRadio_Sleep(double delay){
	if  (delay == 0.0)
		send(createRadioCommand(SET_STATE, SLEEP), "toMacModule");
	else
		sendDelayed(createRadioCommand(SET_STATE, SLEEP), delay, "toMacModule");
}

//================================================================
//    setRadio_Sleep
//================================================================
void FlyForeverRouting::setRadio_TXPowerLevel(int powerLevel, double delay){
	if(delay == 0.0)	
		send(createRadioCommand(SET_TX_OUTPUT, powerLevel), "toMacModule");
	else
		sendDelayed(createRadioCommand(SET_TX_OUTPUT, powerLevel), delay, "toMacModule");
}

//================================================================
//    to collect statistics
//================================================================
void FlyForeverRouting::statistics(int src, int dest, int videoId, int hops, double mos, int index){
	bool included = false;
	for (int i=videoStatistics.size()-1; i>=0; i--){
		if (videoStatistics[i].nodeId == src && videoStatistics[i].videoId == videoId){
			included = true;
			break;
		}
	}
	if (!included){
		requestData temp;
		temp.nodeId = src;
		temp.videoId = videoId;
		temp.time = SIMTIME_DBL(simTime());
		temp.nRecPkts = 0;
		temp.hopSum = 0;
		temp.frames = 0;
		videoStatistics.push_back(temp);
	}

	for (int i=videoStatistics.size()-1; i>=0; i--){
		if (videoStatistics[i].nodeId == src && videoStatistics[i].videoId == videoId){
			switch (index) {
				case FlyForever_STATISTICS_RECEIVED_PACKET:{
					videoStatistics[i].hopSum += hops;
					videoStatistics[i].nRecPkts += 1;
					break;
				}
				case FlyForever_STATISTICS_FRAMES:{
					videoStatistics[i].frames = mos;
					break;
				}
				case FlyForever_STATISTICS_ROUTE:{
					videoStatistics[i].hopSum ++;
					break;
				}
			}
			break;
		}
	}
}

//================================================================
//    getGCSlocation
//================================================================
FlyForeverLocationInfo FlyForeverRouting::getGCSlocation(){
	FlyForeverLocationInfo temp;
	temp.x = 4; 
	temp.y = 200;
	return temp;
}

//================================================================
//    loadControlerList
//================================================================
void FlyForeverRouting::loadControlerList(){
	controllerInfo temp;
	temp.controllerId = 20;
	temp.controllerLocation.x = 100;
	temp.controllerLocation.y = 100;
	controllerList.push_back(temp);
	temp.controllerId = 21;
	temp.controllerLocation.x = 65;
	temp.controllerLocation.y = 65;
	controllerList.push_back(temp);
	temp.controllerId = 22;
	temp.controllerLocation.x = 65;
	temp.controllerLocation.y = 135;
	controllerList.push_back(temp);
	temp.controllerId = 23;
	temp.controllerLocation.x = 135;
	temp.controllerLocation.y = 65;
	controllerList.push_back(temp);
	temp.controllerId = 24;
	temp.controllerLocation.x = 135;
	temp.controllerLocation.y = 135;
	controllerList.push_back(temp);
}

//================================================================
//    loadEventList
//================================================================
void FlyForeverRouting::loadEventList(){
	idEvent = 0;
	eventInfo temp;
	temp.time = 5.160;
	temp.eventLocation.x = 184.938;
	temp.eventLocation.y = 189.376;
	eventList.push_back(temp);
	temp.time = 25.161;
	temp.eventLocation.x = 173.196;
	temp.eventLocation.y = 128.6452;
	eventList.push_back(temp);
	temp.time = 45.159;
	temp.eventLocation.x = 131.702;
	temp.eventLocation.y = 125.469;
	eventList.push_back(temp);
	temp.time = 65.160;
	temp.eventLocation.x = 136.806;
	temp.eventLocation.y = 190.478;
	eventList.push_back(temp);
	temp.time = 85.162;
	temp.eventLocation.x = 101.795;
	temp.eventLocation.y = 144.246;
	eventList.push_back(temp);
	temp.time = 105.160;
	temp.eventLocation.x = 182.57;
	temp.eventLocation.y = 189.869;
	eventList.push_back(temp);
	temp.time = 125.166;
	temp.eventLocation.x = 152.171;
	temp.eventLocation.y = 129.6704;
	eventList.push_back(temp);
	temp.time = 145.159;
	temp.eventLocation.x = 130.677;
	temp.eventLocation.y = 126.494;
	eventList.push_back(temp);
	temp.time = 165.162;
	temp.eventLocation.x = 161.105;
	temp.eventLocation.y = 192.3;
	eventList.push_back(temp);
	temp.time = 185.166;
	temp.eventLocation.x = 114.508;
	temp.eventLocation.y = 152.731;
	eventList.push_back(temp);
	temp.time = 205.166;
	temp.eventLocation.x = 54.508;
	temp.eventLocation.y = 20.731;
	eventList.push_back(temp);
	temp.time = 225.166;
	temp.eventLocation.x = 84.508;
	temp.eventLocation.y = 82.731;
	eventList.push_back(temp);
	temp.time = 245.166;
	temp.eventLocation.x = 184.508;
	temp.eventLocation.y = 152.731;
	eventList.push_back(temp);
	temp.time = 265.166;
	temp.eventLocation.x = 64.508;
	temp.eventLocation.y = 162.731;
	eventList.push_back(temp);
	temp.time = 285.166;
	temp.eventLocation.x = 104.508;
	temp.eventLocation.y = 172.731;
	eventList.push_back(temp);
	temp.time = 305.166;
	temp.eventLocation.x = 74.508;
	temp.eventLocation.y = 72.731;
	eventList.push_back(temp);
	temp.time = 325.166;
	temp.eventLocation.x = 154.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 345.166;
	temp.eventLocation.x = 104.508;
	temp.eventLocation.y = 162.731;
	eventList.push_back(temp);
	temp.time = 365.166;
	temp.eventLocation.x = 124.508;
	temp.eventLocation.y = 172.731;
	eventList.push_back(temp);
	temp.time = 385.166;
	temp.eventLocation.x = 44.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 405.160;
	temp.eventLocation.x = 184.938;
	temp.eventLocation.y = 189.376;
	eventList.push_back(temp);
	temp.time = 425.161;
	temp.eventLocation.x = 173.196;
	temp.eventLocation.y = 128.6452;
	eventList.push_back(temp);
	temp.time = 445.159;
	temp.eventLocation.x = 131.702;
	temp.eventLocation.y = 125.469;
	eventList.push_back(temp);
	temp.time = 465.160;
	temp.eventLocation.x = 136.806;
	temp.eventLocation.y = 190.478;
	eventList.push_back(temp);
	temp.time = 485.162;
	temp.eventLocation.x = 101.795;
	temp.eventLocation.y = 144.246;
	eventList.push_back(temp);
	temp.time = 505.160;
	temp.eventLocation.x = 182.57;
	temp.eventLocation.y = 189.869;
	eventList.push_back(temp);
	temp.time = 525.166;
	temp.eventLocation.x = 152.171;
	temp.eventLocation.y = 129.6704;
	eventList.push_back(temp);
	temp.time = 545.159;
	temp.eventLocation.x = 130.677;
	temp.eventLocation.y = 126.494;
	eventList.push_back(temp);
	temp.time = 565.162;
	temp.eventLocation.x = 161.105;
	temp.eventLocation.y = 192.3;
	eventList.push_back(temp);
	temp.time = 585.166;
	temp.eventLocation.x = 114.508;
	temp.eventLocation.y = 152.731;
	eventList.push_back(temp);
	temp.time = 605.166;
	temp.eventLocation.x = 54.508;
	temp.eventLocation.y = 20.731;
	eventList.push_back(temp);
	temp.time = 625.166;
	temp.eventLocation.x = 84.508;
	temp.eventLocation.y = 82.731;
	eventList.push_back(temp);
	temp.time = 645.166;
	temp.eventLocation.x = 184.508;
	temp.eventLocation.y = 152.731;
	eventList.push_back(temp);
	temp.time = 665.166;
	temp.eventLocation.x = 64.508;
	temp.eventLocation.y = 162.731;
	eventList.push_back(temp);
	temp.time = 685.166;
	temp.eventLocation.x = 104.508;
	temp.eventLocation.y = 172.731;
	eventList.push_back(temp);
	temp.time = 705.166;
	temp.eventLocation.x = 74.508;
	temp.eventLocation.y = 72.731;
	eventList.push_back(temp);
	temp.time = 725.166;
	temp.eventLocation.x = 154.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 745.166;
	temp.eventLocation.x = 104.508;
	temp.eventLocation.y = 162.731;
	eventList.push_back(temp);
	temp.time = 765.166;
	temp.eventLocation.x = 124.508;
	temp.eventLocation.y = 172.731;
	eventList.push_back(temp);
	temp.time = 785.166;
	temp.eventLocation.x = 44.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 805.166;
	temp.eventLocation.x = 74.508;
	temp.eventLocation.y = 72.731;
	eventList.push_back(temp);
	temp.time = 825.166;
	temp.eventLocation.x = 154.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 845.166;
	temp.eventLocation.x = 104.508;
	temp.eventLocation.y = 162.731;
	eventList.push_back(temp);
	temp.time = 865.166;
	temp.eventLocation.x = 124.508;
	temp.eventLocation.y = 172.731;
	eventList.push_back(temp);
	temp.time = 885.166;
	temp.eventLocation.x = 44.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 905.160;
	temp.eventLocation.x = 184.938;
	temp.eventLocation.y = 189.376;
	eventList.push_back(temp);
	temp.time = 925.161;
	temp.eventLocation.x = 173.196;
	temp.eventLocation.y = 128.6452;
	eventList.push_back(temp);
	temp.time = 945.159;
	temp.eventLocation.x = 131.702;
	temp.eventLocation.y = 125.469;
	eventList.push_back(temp);
	temp.time = 965.160;
	temp.eventLocation.x = 136.806;
	temp.eventLocation.y = 190.478;
	eventList.push_back(temp);
	temp.time = 985.162;
	temp.eventLocation.x = 101.795;
	temp.eventLocation.y = 144.246;
	eventList.push_back(temp);
	temp.time = 1005.166;
	temp.eventLocation.x = 74.508;
	temp.eventLocation.y = 72.731;
	eventList.push_back(temp);
	temp.time = 1025.166;
	temp.eventLocation.x = 154.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 1045.166;
	temp.eventLocation.x = 104.508;
	temp.eventLocation.y = 162.731;
	eventList.push_back(temp);
	temp.time = 1065.166;
	temp.eventLocation.x = 124.508;
	temp.eventLocation.y = 172.731;
	eventList.push_back(temp);
	temp.time = 1085.166;
	temp.eventLocation.x = 44.508;
	temp.eventLocation.y = 52.731;
	eventList.push_back(temp);
	temp.time = 1105.160;
	temp.eventLocation.x = 184.938;
	temp.eventLocation.y = 189.376;
	eventList.push_back(temp);
	temp.time = 1125.161;
	temp.eventLocation.x = 173.196;
	temp.eventLocation.y = 128.6452;
	eventList.push_back(temp);
	temp.time = 1145.159;
	temp.eventLocation.x = 131.702;
	temp.eventLocation.y = 125.469;
	eventList.push_back(temp);
	temp.time = 1165.160;
	temp.eventLocation.x = 136.806;
	temp.eventLocation.y = 190.478;
	eventList.push_back(temp);
	temp.time = 1185.162;
	temp.eventLocation.x = 101.795;
	temp.eventLocation.y = 144.246;
	eventList.push_back(temp);
}

//================================================================
//    FlyForever_sort_buffer
//================================================================
bool FlyForever_sort_buffer(pktList a, pktList b){
	return (a.seqNumber < b.seqNumber);
}

//================================================================
//    LinGO_sort_node_list
//================================================================
bool FlyForever_sort_node(nodeCInfo a, nodeCInfo b){
	return (a.avg < b.avg);
}

//================================================================
//    LinGO_sort_node_list
//================================================================
bool FlyForever_sort_time(nodeCInfo a, nodeCInfo b){
	return (a.timeMove < b.timeMove);
}
