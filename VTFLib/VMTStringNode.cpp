/*
 * VTFLib
 * Copyright (C) 2005-2010 Neil Jedrzejewski & Ryan Gregg

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later
 * version.
 */

#include "VMTStringNode.h"

using namespace VTFLib::Nodes;

CVMTStringNode::CVMTStringNode(const char *name) : CVMTValueNode(name)
{
	this->mValue = new char[1];
	*this->mValue = '\0';
}

CVMTStringNode::CVMTStringNode(const char *name, const char *value) : CVMTValueNode(name)
{
	this->mValue = new char[strlen(value) + 1];
	strcpy(this->mValue, value);
}

CVMTStringNode::CVMTStringNode(const CVMTStringNode &other) : CVMTValueNode(other.GetName())
{
	this->mValue = new char[strlen(other.mValue) + 1];
	strcpy(this->mValue, other.mValue);
}

CVMTStringNode::~CVMTStringNode()
{
	delete[] this->mValue;
}

void CVMTStringNode::SetValue(const char *value)
{
	delete[] this->mValue;
	this->mValue = new char[strlen(value) + 1];
	strcpy(this->mValue, value);
}

const char *CVMTStringNode::GetValue() const
{
	return this->mValue;
}

VMTNodeType CVMTStringNode::GetType() const
{
	return NODE_TYPE_STRING;
}

CVMTNode *CVMTStringNode::Clone() const
{
	return new CVMTStringNode(*this);
}