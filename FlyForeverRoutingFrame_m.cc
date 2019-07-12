//
// Generated file, do not edit! Created by nedtool 4.6 from src/node/communication/routing/FlyForever/FlyForeverRoutingFrame.msg.
//

// Disable warnings about unused variables, empty switch stmts, etc:
#ifdef _MSC_VER
#  pragma warning(disable:4101)
#  pragma warning(disable:4065)
#endif

#include <iostream>
#include <sstream>
#include "FlyForeverRoutingFrame_m.h"

USING_NAMESPACE


// Another default rule (prevents compiler from choosing base class' doPacking())
template<typename T>
void doPacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doPacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}

template<typename T>
void doUnpacking(cCommBuffer *, T& t) {
    throw cRuntimeError("Parsim error: no doUnpacking() function for type %s or its base class (check .msg and _m.cc/h files!)",opp_typename(typeid(t)));
}




// Template rule for outputting std::vector<T> types
template<typename T, typename A>
inline std::ostream& operator<<(std::ostream& out, const std::vector<T,A>& vec)
{
    out.put('{');
    for(typename std::vector<T,A>::const_iterator it = vec.begin(); it != vec.end(); ++it)
    {
        if (it != vec.begin()) {
            out.put(','); out.put(' ');
        }
        out << *it;
    }
    out.put('}');
    
    char buf[32];
    sprintf(buf, " (size=%u)", (unsigned int)vec.size());
    out.write(buf, strlen(buf));
    return out;
}

// Template rule which fires if a struct or class doesn't have operator<<
template<typename T>
inline std::ostream& operator<<(std::ostream& out,const T&) {return out;}

EXECUTE_ON_STARTUP(
    cEnum *e = cEnum::find("FlyForeverRouting_FrameType");
    if (!e) enums.getInstance()->add(e = new cEnum("FlyForeverRouting_FrameType"));
    e->insert(FlyForever_MULTIMEDIA_PKT, "FlyForever_MULTIMEDIA_PKT");
    e->insert(FlyForever_ADV_MSG, "FlyForever_ADV_MSG");
    e->insert(FlyForever_SN_MSG, "FlyForever_SN_MSG");
    e->insert(FlyForever_RN_MSG, "FlyForever_RN_MSG");
    e->insert(FlyForever_ACK_MSG, "FlyForever_ACK_MSG");
    e->insert(FlyForever_REPO_MSG, "FlyForever_REPO_MSG");
);

FlyForeverLocationInfo::FlyForeverLocationInfo()
{
    x = 0;
    y = 0;
}

void doPacking(cCommBuffer *b, FlyForeverLocationInfo& a)
{
    doPacking(b,a.x);
    doPacking(b,a.y);
}

void doUnpacking(cCommBuffer *b, FlyForeverLocationInfo& a)
{
    doUnpacking(b,a.x);
    doUnpacking(b,a.y);
}

class FlyForeverLocationInfoDescriptor : public cClassDescriptor
{
  public:
    FlyForeverLocationInfoDescriptor();
    virtual ~FlyForeverLocationInfoDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(FlyForeverLocationInfoDescriptor);

FlyForeverLocationInfoDescriptor::FlyForeverLocationInfoDescriptor() : cClassDescriptor("FlyForeverLocationInfo", "")
{
}

FlyForeverLocationInfoDescriptor::~FlyForeverLocationInfoDescriptor()
{
}

bool FlyForeverLocationInfoDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<FlyForeverLocationInfo *>(obj)!=NULL;
}

const char *FlyForeverLocationInfoDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int FlyForeverLocationInfoDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 2+basedesc->getFieldCount(object) : 2;
}

unsigned int FlyForeverLocationInfoDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<2) ? fieldTypeFlags[field] : 0;
}

const char *FlyForeverLocationInfoDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "x",
        "y",
    };
    return (field>=0 && field<2) ? fieldNames[field] : NULL;
}

int FlyForeverLocationInfoDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='x' && strcmp(fieldName, "x")==0) return base+0;
    if (fieldName[0]=='y' && strcmp(fieldName, "y")==0) return base+1;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *FlyForeverLocationInfoDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "double",
        "double",
    };
    return (field>=0 && field<2) ? fieldTypeStrings[field] : NULL;
}

const char *FlyForeverLocationInfoDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    }
}

int FlyForeverLocationInfoDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverLocationInfo *pp = (FlyForeverLocationInfo *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string FlyForeverLocationInfoDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverLocationInfo *pp = (FlyForeverLocationInfo *)object; (void)pp;
    switch (field) {
        case 0: return double2string(pp->x);
        case 1: return double2string(pp->y);
        default: return "";
    }
}

bool FlyForeverLocationInfoDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverLocationInfo *pp = (FlyForeverLocationInfo *)object; (void)pp;
    switch (field) {
        case 0: pp->x = string2double(value); return true;
        case 1: pp->y = string2double(value); return true;
        default: return false;
    }
}

const char *FlyForeverLocationInfoDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        default: return NULL;
    };
}

void *FlyForeverLocationInfoDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverLocationInfo *pp = (FlyForeverLocationInfo *)object; (void)pp;
    switch (field) {
        default: return NULL;
    }
}

Register_Class(FlyForeverRoutingPacket);

FlyForeverRoutingPacket::FlyForeverRoutingPacket(const char *name, int kind) : ::RoutingPacket(name,kind)
{
    this->FlyForeverRoutingPacketKind_var = 0;
    this->nextHop_var = 0;
    this->nextHopRoute_var = 0;
    this->sourceRoute_var = 0;
    this->RE_var = 0;
    this->speed_var = 0;
    this->traject_var = 0;
    this->data_var = 0;
    this->replacement_var = 0;
    this->recharging_var = 0;
    this->startVideo_var = 0;
}

FlyForeverRoutingPacket::FlyForeverRoutingPacket(const FlyForeverRoutingPacket& other) : ::RoutingPacket(other)
{
    copy(other);
}

FlyForeverRoutingPacket::~FlyForeverRoutingPacket()
{
}

FlyForeverRoutingPacket& FlyForeverRoutingPacket::operator=(const FlyForeverRoutingPacket& other)
{
    if (this==&other) return *this;
    ::RoutingPacket::operator=(other);
    copy(other);
    return *this;
}

void FlyForeverRoutingPacket::copy(const FlyForeverRoutingPacket& other)
{
    this->FlyForeverRoutingPacketKind_var = other.FlyForeverRoutingPacketKind_var;
    this->currentLocation_var = other.currentLocation_var;
    this->eventLocation_var = other.eventLocation_var;
    this->idealLocation_var = other.idealLocation_var;
    this->nextHop_var = other.nextHop_var;
    this->nextHopRoute_var = other.nextHopRoute_var;
    this->sourceRoute_var = other.sourceRoute_var;
    this->RE_var = other.RE_var;
    this->speed_var = other.speed_var;
    this->traject_var = other.traject_var;
    this->data_var = other.data_var;
    this->replacement_var = other.replacement_var;
    this->recharging_var = other.recharging_var;
    this->startVideo_var = other.startVideo_var;
}

void FlyForeverRoutingPacket::parsimPack(cCommBuffer *b)
{
    ::RoutingPacket::parsimPack(b);
    doPacking(b,this->FlyForeverRoutingPacketKind_var);
    doPacking(b,this->currentLocation_var);
    doPacking(b,this->eventLocation_var);
    doPacking(b,this->idealLocation_var);
    doPacking(b,this->nextHop_var);
    doPacking(b,this->nextHopRoute_var);
    doPacking(b,this->sourceRoute_var);
    doPacking(b,this->RE_var);
    doPacking(b,this->speed_var);
    doPacking(b,this->traject_var);
    doPacking(b,this->data_var);
    doPacking(b,this->replacement_var);
    doPacking(b,this->recharging_var);
    doPacking(b,this->startVideo_var);
}

void FlyForeverRoutingPacket::parsimUnpack(cCommBuffer *b)
{
    ::RoutingPacket::parsimUnpack(b);
    doUnpacking(b,this->FlyForeverRoutingPacketKind_var);
    doUnpacking(b,this->currentLocation_var);
    doUnpacking(b,this->eventLocation_var);
    doUnpacking(b,this->idealLocation_var);
    doUnpacking(b,this->nextHop_var);
    doUnpacking(b,this->nextHopRoute_var);
    doUnpacking(b,this->sourceRoute_var);
    doUnpacking(b,this->RE_var);
    doUnpacking(b,this->speed_var);
    doUnpacking(b,this->traject_var);
    doUnpacking(b,this->data_var);
    doUnpacking(b,this->replacement_var);
    doUnpacking(b,this->recharging_var);
    doUnpacking(b,this->startVideo_var);
}

int FlyForeverRoutingPacket::getFlyForeverRoutingPacketKind() const
{
    return FlyForeverRoutingPacketKind_var;
}

void FlyForeverRoutingPacket::setFlyForeverRoutingPacketKind(int FlyForeverRoutingPacketKind)
{
    this->FlyForeverRoutingPacketKind_var = FlyForeverRoutingPacketKind;
}

FlyForeverLocationInfo& FlyForeverRoutingPacket::getCurrentLocation()
{
    return currentLocation_var;
}

void FlyForeverRoutingPacket::setCurrentLocation(const FlyForeverLocationInfo& currentLocation)
{
    this->currentLocation_var = currentLocation;
}

FlyForeverLocationInfo& FlyForeverRoutingPacket::getEventLocation()
{
    return eventLocation_var;
}

void FlyForeverRoutingPacket::setEventLocation(const FlyForeverLocationInfo& eventLocation)
{
    this->eventLocation_var = eventLocation;
}

FlyForeverLocationInfo& FlyForeverRoutingPacket::getIdealLocation()
{
    return idealLocation_var;
}

void FlyForeverRoutingPacket::setIdealLocation(const FlyForeverLocationInfo& idealLocation)
{
    this->idealLocation_var = idealLocation;
}

const char * FlyForeverRoutingPacket::getNextHop() const
{
    return nextHop_var.c_str();
}

void FlyForeverRoutingPacket::setNextHop(const char * nextHop)
{
    this->nextHop_var = nextHop;
}

const char * FlyForeverRoutingPacket::getNextHopRoute() const
{
    return nextHopRoute_var.c_str();
}

void FlyForeverRoutingPacket::setNextHopRoute(const char * nextHopRoute)
{
    this->nextHopRoute_var = nextHopRoute;
}

int FlyForeverRoutingPacket::getSourceRoute() const
{
    return sourceRoute_var;
}

void FlyForeverRoutingPacket::setSourceRoute(int sourceRoute)
{
    this->sourceRoute_var = sourceRoute;
}

double FlyForeverRoutingPacket::getRE() const
{
    return RE_var;
}

void FlyForeverRoutingPacket::setRE(double RE)
{
    this->RE_var = RE;
}

double FlyForeverRoutingPacket::getSpeed() const
{
    return speed_var;
}

void FlyForeverRoutingPacket::setSpeed(double speed)
{
    this->speed_var = speed;
}

int FlyForeverRoutingPacket::getTraject() const
{
    return traject_var;
}

void FlyForeverRoutingPacket::setTraject(int traject)
{
    this->traject_var = traject;
}

double FlyForeverRoutingPacket::getData() const
{
    return data_var;
}

void FlyForeverRoutingPacket::setData(double data)
{
    this->data_var = data;
}

bool FlyForeverRoutingPacket::getReplacement() const
{
    return replacement_var;
}

void FlyForeverRoutingPacket::setReplacement(bool replacement)
{
    this->replacement_var = replacement;
}

bool FlyForeverRoutingPacket::getRecharging() const
{
    return recharging_var;
}

void FlyForeverRoutingPacket::setRecharging(bool recharging)
{
    this->recharging_var = recharging;
}

double FlyForeverRoutingPacket::getStartVideo() const
{
    return startVideo_var;
}

void FlyForeverRoutingPacket::setStartVideo(double startVideo)
{
    this->startVideo_var = startVideo;
}

class FlyForeverRoutingPacketDescriptor : public cClassDescriptor
{
  public:
    FlyForeverRoutingPacketDescriptor();
    virtual ~FlyForeverRoutingPacketDescriptor();

    virtual bool doesSupport(cObject *obj) const;
    virtual const char *getProperty(const char *propertyname) const;
    virtual int getFieldCount(void *object) const;
    virtual const char *getFieldName(void *object, int field) const;
    virtual int findField(void *object, const char *fieldName) const;
    virtual unsigned int getFieldTypeFlags(void *object, int field) const;
    virtual const char *getFieldTypeString(void *object, int field) const;
    virtual const char *getFieldProperty(void *object, int field, const char *propertyname) const;
    virtual int getArraySize(void *object, int field) const;

    virtual std::string getFieldAsString(void *object, int field, int i) const;
    virtual bool setFieldAsString(void *object, int field, int i, const char *value) const;

    virtual const char *getFieldStructName(void *object, int field) const;
    virtual void *getFieldStructPointer(void *object, int field, int i) const;
};

Register_ClassDescriptor(FlyForeverRoutingPacketDescriptor);

FlyForeverRoutingPacketDescriptor::FlyForeverRoutingPacketDescriptor() : cClassDescriptor("FlyForeverRoutingPacket", "RoutingPacket")
{
}

FlyForeverRoutingPacketDescriptor::~FlyForeverRoutingPacketDescriptor()
{
}

bool FlyForeverRoutingPacketDescriptor::doesSupport(cObject *obj) const
{
    return dynamic_cast<FlyForeverRoutingPacket *>(obj)!=NULL;
}

const char *FlyForeverRoutingPacketDescriptor::getProperty(const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? basedesc->getProperty(propertyname) : NULL;
}

int FlyForeverRoutingPacketDescriptor::getFieldCount(void *object) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    return basedesc ? 14+basedesc->getFieldCount(object) : 14;
}

unsigned int FlyForeverRoutingPacketDescriptor::getFieldTypeFlags(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeFlags(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static unsigned int fieldTypeFlags[] = {
        FD_ISEDITABLE,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISCOMPOUND,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
        FD_ISEDITABLE,
    };
    return (field>=0 && field<14) ? fieldTypeFlags[field] : 0;
}

const char *FlyForeverRoutingPacketDescriptor::getFieldName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldNames[] = {
        "FlyForeverRoutingPacketKind",
        "currentLocation",
        "eventLocation",
        "idealLocation",
        "nextHop",
        "nextHopRoute",
        "sourceRoute",
        "RE",
        "speed",
        "traject",
        "data",
        "replacement",
        "recharging",
        "startVideo",
    };
    return (field>=0 && field<14) ? fieldNames[field] : NULL;
}

int FlyForeverRoutingPacketDescriptor::findField(void *object, const char *fieldName) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    int base = basedesc ? basedesc->getFieldCount(object) : 0;
    if (fieldName[0]=='F' && strcmp(fieldName, "FlyForeverRoutingPacketKind")==0) return base+0;
    if (fieldName[0]=='c' && strcmp(fieldName, "currentLocation")==0) return base+1;
    if (fieldName[0]=='e' && strcmp(fieldName, "eventLocation")==0) return base+2;
    if (fieldName[0]=='i' && strcmp(fieldName, "idealLocation")==0) return base+3;
    if (fieldName[0]=='n' && strcmp(fieldName, "nextHop")==0) return base+4;
    if (fieldName[0]=='n' && strcmp(fieldName, "nextHopRoute")==0) return base+5;
    if (fieldName[0]=='s' && strcmp(fieldName, "sourceRoute")==0) return base+6;
    if (fieldName[0]=='R' && strcmp(fieldName, "RE")==0) return base+7;
    if (fieldName[0]=='s' && strcmp(fieldName, "speed")==0) return base+8;
    if (fieldName[0]=='t' && strcmp(fieldName, "traject")==0) return base+9;
    if (fieldName[0]=='d' && strcmp(fieldName, "data")==0) return base+10;
    if (fieldName[0]=='r' && strcmp(fieldName, "replacement")==0) return base+11;
    if (fieldName[0]=='r' && strcmp(fieldName, "recharging")==0) return base+12;
    if (fieldName[0]=='s' && strcmp(fieldName, "startVideo")==0) return base+13;
    return basedesc ? basedesc->findField(object, fieldName) : -1;
}

const char *FlyForeverRoutingPacketDescriptor::getFieldTypeString(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldTypeString(object, field);
        field -= basedesc->getFieldCount(object);
    }
    static const char *fieldTypeStrings[] = {
        "int",
        "FlyForeverLocationInfo",
        "FlyForeverLocationInfo",
        "FlyForeverLocationInfo",
        "string",
        "string",
        "int",
        "double",
        "double",
        "int",
        "double",
        "bool",
        "bool",
        "double",
    };
    return (field>=0 && field<14) ? fieldTypeStrings[field] : NULL;
}

const char *FlyForeverRoutingPacketDescriptor::getFieldProperty(void *object, int field, const char *propertyname) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldProperty(object, field, propertyname);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 0:
            if (!strcmp(propertyname,"enum")) return "FlyForeverRouting_FrameType";
            return NULL;
        default: return NULL;
    }
}

int FlyForeverRoutingPacketDescriptor::getArraySize(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getArraySize(object, field);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverRoutingPacket *pp = (FlyForeverRoutingPacket *)object; (void)pp;
    switch (field) {
        default: return 0;
    }
}

std::string FlyForeverRoutingPacketDescriptor::getFieldAsString(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldAsString(object,field,i);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverRoutingPacket *pp = (FlyForeverRoutingPacket *)object; (void)pp;
    switch (field) {
        case 0: return long2string(pp->getFlyForeverRoutingPacketKind());
        case 1: {std::stringstream out; out << pp->getCurrentLocation(); return out.str();}
        case 2: {std::stringstream out; out << pp->getEventLocation(); return out.str();}
        case 3: {std::stringstream out; out << pp->getIdealLocation(); return out.str();}
        case 4: return oppstring2string(pp->getNextHop());
        case 5: return oppstring2string(pp->getNextHopRoute());
        case 6: return long2string(pp->getSourceRoute());
        case 7: return double2string(pp->getRE());
        case 8: return double2string(pp->getSpeed());
        case 9: return long2string(pp->getTraject());
        case 10: return double2string(pp->getData());
        case 11: return bool2string(pp->getReplacement());
        case 12: return bool2string(pp->getRecharging());
        case 13: return double2string(pp->getStartVideo());
        default: return "";
    }
}

bool FlyForeverRoutingPacketDescriptor::setFieldAsString(void *object, int field, int i, const char *value) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->setFieldAsString(object,field,i,value);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverRoutingPacket *pp = (FlyForeverRoutingPacket *)object; (void)pp;
    switch (field) {
        case 0: pp->setFlyForeverRoutingPacketKind(string2long(value)); return true;
        case 4: pp->setNextHop((value)); return true;
        case 5: pp->setNextHopRoute((value)); return true;
        case 6: pp->setSourceRoute(string2long(value)); return true;
        case 7: pp->setRE(string2double(value)); return true;
        case 8: pp->setSpeed(string2double(value)); return true;
        case 9: pp->setTraject(string2long(value)); return true;
        case 10: pp->setData(string2double(value)); return true;
        case 11: pp->setReplacement(string2bool(value)); return true;
        case 12: pp->setRecharging(string2bool(value)); return true;
        case 13: pp->setStartVideo(string2double(value)); return true;
        default: return false;
    }
}

const char *FlyForeverRoutingPacketDescriptor::getFieldStructName(void *object, int field) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructName(object, field);
        field -= basedesc->getFieldCount(object);
    }
    switch (field) {
        case 1: return opp_typename(typeid(FlyForeverLocationInfo));
        case 2: return opp_typename(typeid(FlyForeverLocationInfo));
        case 3: return opp_typename(typeid(FlyForeverLocationInfo));
        default: return NULL;
    };
}

void *FlyForeverRoutingPacketDescriptor::getFieldStructPointer(void *object, int field, int i) const
{
    cClassDescriptor *basedesc = getBaseClassDescriptor();
    if (basedesc) {
        if (field < basedesc->getFieldCount(object))
            return basedesc->getFieldStructPointer(object, field, i);
        field -= basedesc->getFieldCount(object);
    }
    FlyForeverRoutingPacket *pp = (FlyForeverRoutingPacket *)object; (void)pp;
    switch (field) {
        case 1: return (void *)(&pp->getCurrentLocation()); break;
        case 2: return (void *)(&pp->getEventLocation()); break;
        case 3: return (void *)(&pp->getIdealLocation()); break;
        default: return NULL;
    }
}


