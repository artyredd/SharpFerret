#pragma once

// MACRO: short for destination->member = source->member 
#define CopyMember(source,destination,member) destination->member = source->member

// MACRO: short for destination.field = source.field
#define CopyField(source,destination,field) destination.field = source.field