//
// Generated file, do not edit! Created by nedtool 4.6 from src/node/communication/routing/FlyForever/FlyForeverRoutingFrame.msg.
//

#ifndef _FLYFOREVERROUTINGFRAME_M_H_
#define _FLYFOREVERROUTINGFRAME_M_H_

#include <omnetpp.h>

// nedtool version check
#define MSGC_VERSION 0x0406
#if (MSGC_VERSION!=OMNETPP_VERSION)
#    error Version mismatch! Probably this file was generated by an earlier version of nedtool: 'make clean' should help.
#endif



// cplusplus {{
#include "RoutingPacket_m.h"
// }}

/**
 * Enum generated from <tt>src/node/communication/routing/FlyForever/FlyForeverRoutingFrame.msg:29</tt> by nedtool.
 * <pre>
 * enum FlyForeverRouting_FrameType
 * {
 * 
 *     FlyForever_MULTIMEDIA_PKT = 0100;
 *     FlyForever_ADV_MSG = 0101;
 *     FlyForever_SN_MSG = 0102;
 *     FlyForever_RN_MSG = 0103;
 *     FlyForever_ACK_MSG = 0104;
 *     FlyForever_REPO_MSG = 0105;
 * }
 * </pre>
 */
enum FlyForeverRouting_FrameType {
    FlyForever_MULTIMEDIA_PKT = 0100,
    FlyForever_ADV_MSG = 0101,
    FlyForever_SN_MSG = 0102,
    FlyForever_RN_MSG = 0103,
    FlyForever_ACK_MSG = 0104,
    FlyForever_REPO_MSG = 0105
};

/**
 * Struct generated from src/node/communication/routing/FlyForever/FlyForeverRoutingFrame.msg:38 by nedtool.
 */
struct FlyForeverLocationInfo
{
    FlyForeverLocationInfo();
    double x;
    double y;
};

void doPacking(cCommBuffer *b, FlyForeverLocationInfo& a);
void doUnpacking(cCommBuffer *b, FlyForeverLocationInfo& a);

/**
 * Class generated from <tt>src/node/communication/routing/FlyForever/FlyForeverRoutingFrame.msg:43</tt> by nedtool.
 * <pre>
 * packet FlyForeverRoutingPacket extends RoutingPacket
 * {
 *     int FlyForeverRoutingPacketKind @enum(FlyForeverRouting_FrameType);
 *     FlyForeverLocationInfo currentLocation;
 *     FlyForeverLocationInfo eventLocation;
 *     FlyForeverLocationInfo idealLocation;
 *     string nextHop;
 *     string nextHopRoute;
 *     int sourceRoute;
 *     double RE;
 *     double speed;
 *     int traject;
 *     double data;
 *     bool replacement;
 *     bool recharging;
 *     double startVideo;
 * }
 * </pre>
 */
class FlyForeverRoutingPacket : public ::RoutingPacket
{
  protected:
    int FlyForeverRoutingPacketKind_var;
    FlyForeverLocationInfo currentLocation_var;
    FlyForeverLocationInfo eventLocation_var;
    FlyForeverLocationInfo idealLocation_var;
    opp_string nextHop_var;
    opp_string nextHopRoute_var;
    int sourceRoute_var;
    double RE_var;
    double speed_var;
    int traject_var;
    double data_var;
    bool replacement_var;
    bool recharging_var;
    double startVideo_var;

  private:
    void copy(const FlyForeverRoutingPacket& other);

  protected:
    // protected and unimplemented operator==(), to prevent accidental usage
    bool operator==(const FlyForeverRoutingPacket&);

  public:
    FlyForeverRoutingPacket(const char *name=NULL, int kind=0);
    FlyForeverRoutingPacket(const FlyForeverRoutingPacket& other);
    virtual ~FlyForeverRoutingPacket();
    FlyForeverRoutingPacket& operator=(const FlyForeverRoutingPacket& other);
    virtual FlyForeverRoutingPacket *dup() const {return new FlyForeverRoutingPacket(*this);}
    virtual void parsimPack(cCommBuffer *b);
    virtual void parsimUnpack(cCommBuffer *b);

    // field getter/setter methods
    virtual int getFlyForeverRoutingPacketKind() const;
    virtual void setFlyForeverRoutingPacketKind(int FlyForeverRoutingPacketKind);
    virtual FlyForeverLocationInfo& getCurrentLocation();
    virtual const FlyForeverLocationInfo& getCurrentLocation() const {return const_cast<FlyForeverRoutingPacket*>(this)->getCurrentLocation();}
    virtual void setCurrentLocation(const FlyForeverLocationInfo& currentLocation);
    virtual FlyForeverLocationInfo& getEventLocation();
    virtual const FlyForeverLocationInfo& getEventLocation() const {return const_cast<FlyForeverRoutingPacket*>(this)->getEventLocation();}
    virtual void setEventLocation(const FlyForeverLocationInfo& eventLocation);
    virtual FlyForeverLocationInfo& getIdealLocation();
    virtual const FlyForeverLocationInfo& getIdealLocation() const {return const_cast<FlyForeverRoutingPacket*>(this)->getIdealLocation();}
    virtual void setIdealLocation(const FlyForeverLocationInfo& idealLocation);
    virtual const char * getNextHop() const;
    virtual void setNextHop(const char * nextHop);
    virtual const char * getNextHopRoute() const;
    virtual void setNextHopRoute(const char * nextHopRoute);
    virtual int getSourceRoute() const;
    virtual void setSourceRoute(int sourceRoute);
    virtual double getRE() const;
    virtual void setRE(double RE);
    virtual double getSpeed() const;
    virtual void setSpeed(double speed);
    virtual int getTraject() const;
    virtual void setTraject(int traject);
    virtual double getData() const;
    virtual void setData(double data);
    virtual bool getReplacement() const;
    virtual void setReplacement(bool replacement);
    virtual bool getRecharging() const;
    virtual void setRecharging(bool recharging);
    virtual double getStartVideo() const;
    virtual void setStartVideo(double startVideo);
};

inline void doPacking(cCommBuffer *b, FlyForeverRoutingPacket& obj) {obj.parsimPack(b);}
inline void doUnpacking(cCommBuffer *b, FlyForeverRoutingPacket& obj) {obj.parsimUnpack(b);}


#endif // ifndef _FLYFOREVERROUTINGFRAME_M_H_

