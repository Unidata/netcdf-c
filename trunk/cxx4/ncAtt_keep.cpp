#include "ncAtt.h"
#include "ncGroup.h"
#include "ncCheck.h"
#include <vector>
#include <iostream>

using namespace std;
using namespace netCDF;
  

// destructor  (defined even though it is virtual)
NcAtt::~NcAtt() {}

// assignment operator
NcAtt& NcAtt::operator=(const NcAtt& rhs)
{
  if (&rhs != this) {
    nullObject = rhs.nullObject;
    myName = rhs.myName;
    groupId = rhs.groupId;
    varId =rhs.varId;
  }
  return *this;
}

// Constructor generates a null object.
NcAtt::NcAtt() : 
  nullObject(true) 
{}

// Constructor for non-null instances.
NcAtt::NcAtt(bool nullObject): 
  nullObject(nullObject)
{}

// The copy constructor.
NcAtt::NcAtt(const NcAtt& rhs): 
  nullObject(rhs.nullObject),
  myName(rhs.myName),
  groupId(rhs.groupId),
  varId(rhs.varId)
{}


// equivalence operator
bool NcAtt::operator==(const NcAtt & rhs) const
{
  if(nullObject) 
    return nullObject == rhs.nullObject;
  else
    return myName == rhs.myName && groupId == rhs.groupId && varId == rhs.varId;
}  

//  !=  operator
bool NcAtt::operator!=(const NcAtt & rhs) const
{
  return !(*this == rhs);
}  

// Gets parent group.
netCDF::NcGroup  NcAtt::getParentGroup() const {
  return netCDF::NcGroup(groupId);
}
      

// Returns the attribute type.
NcType  NcAtt::getNcType() const{
  // get the attribute type identifier
  // create an NcType called ncTmpwith this identifier
  // get the NcType called ncTmp from a parent or current group.

  // Gets the NcType object with a given name, searching up the tree until the object is found.
  groupId;
  cout <<"groupID ="<<groupId<<endl;
  NcGroup tmpGroup(groupId);
  NcType(tmpGroup,myName);
  NcType(NcGroup(groupId),myName);
  return netCDF::NcType(NcGroup(groupId),myName);
}

// Gets attribute length.
size_t  NcAtt::getAttLength() const{
  size_t lenp;
  ncCheck(nc_inq_attlen(groupId, varId, myName.c_str(), &lenp),__FILE__,__LINE__,__FUNCTION__);
  return lenp;
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(char* dataValues) const {
  ncCheck(nc_get_att_text(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}


// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned char* dataValues) const {
  ncCheck(nc_get_att_uchar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(signed char* dataValues) const {
  ncCheck(nc_get_att_schar(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(short* dataValues) const {
  ncCheck(nc_get_att_short(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(int* dataValues) const {
  ncCheck(nc_get_att_int(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(long* dataValues) const {
  ncCheck(nc_get_att_long(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(float* dataValues) const {
  ncCheck(nc_get_att_float(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(double* dataValues) const {
  ncCheck(nc_get_att_double(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned short* dataValues) const {
  ncCheck(nc_get_att_ushort(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned int* dataValues) const {
  ncCheck(nc_get_att_uint(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(long long* dataValues) const {
  ncCheck(nc_get_att_longlong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(unsigned long long* dataValues) const {
  ncCheck(nc_get_att_ulonglong(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(char** dataValues) const {
  ncCheck(nc_get_att_string(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

// Gets a netCDF variable attribute.
void NcAtt::getValues(void* dataValues) const {
  ncCheck(nc_get_att(groupId,varId,myName.c_str(),dataValues),__FILE__,__LINE__,__FUNCTION__);
}

