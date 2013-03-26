////////////////////////////////////////////////////////////////////////
// XMLParser.hpp
// Super Mario Project
// Copyright (C) 2011  
// Lionel Joseph lionel.r.joseph@gmail.com
// Olivier Guittonneau openmengine@gmail.com
////////////////////////////////////////////////////////////////////////

#pragma once
#ifndef HPP_XMLPARSER
#define HPP_XMLPARSER

#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/parserInternals.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#include <string>

using namespace std;

namespace SuperMarioProject
{
	class XMLParser abstract
	{
	public :
		virtual ~XMLParser() { }

	protected :
		int validateSchema(const char * XMLSchemaFile_shorterNamename, const char * XMLfile_shorterNamename);
	};
}
#endif