//===-- MICmnLog.h ----------------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//++
// File:		MICmnLog.h
//
// Overview:	CMICmnLog interface.
//
// Environment:	Compilers:	Visual C++ 12.
//							gcc (Ubuntu/Linaro 4.8.1-10ubuntu9) 4.8.1
//				Libraries:	See MIReadmetxt. 
//
// Copyright:	None.
//--

#pragma once

// Third party headers:
#include <map>

// In-house headers:
#include "MIUtilString.h"
#include "MICmnBase.h"
#include "MIUtilSingletonBase.h"

//++ ============================================================================
// Details:	MI common code implementation class. Handle application trace 
//			activity logging. Medium objects derived from the Medium abstract 
///			class are registered with this loggor. The function Write is called 
//			by a client callee to log information. That information is given to
//			registered relevant mediums. The medium file is registered during 
//			*this logs initialization so it will always have a file log for the
//			application.
//			Singleton class.
// Gotchas:	None.
// Authors:	Illya Rudkin 01/02/2012.
// Changes:	None.
//--
class CMICmnLog : public MI::ISingleton< CMICmnLog >
{
	friend MI::ISingleton< CMICmnLog >;

// Enumeration:
public:
	//++
	// Description: Data given to the Logger can be of serveral types. The Logger can be
	//				set at levels of verbosity. Can determine how data is sent to one or
	//				mediums.
	//--
	enum ELogVerbosity
	{											// Descriptions of what 'may' occur, depends ultimately on the medium itself. See the medium.
		eLogVerbosity_FnTrace	= 0x00000004,	// Debug function stack call tracing
		eLogVerbosity_DbgOp		= 0x00000008,	// Send a string to the debugguer for display (not implemented)
		eLogVerbosity_ClientMsg	= 0x00000010,	// A client using MI can insert messages into the log (not implemented)
		eLogVerbosity_Log		= 0x00000020	// Send to only the Log file.
	};

// Class:
public:
	//++
	// Description: Register a medium derived from this interface which will be
	//				called writing log trace data i.e. a file or a console. 
	//				Medium objects registered are not owned by *this logger.
	//--
	class IMedium
	{
	public:
		virtual bool					Initialize( void ) = 0;
		virtual const CMIUtilString &	GetName( void ) const = 0;
		virtual bool					Write( const CMIUtilString & vData, const ELogVerbosity veType ) = 0;
		virtual const CMIUtilString &	GetError( void ) const = 0;
		virtual bool					Shutdown( void ) = 0;

		// Not part of the interface, ignore
		//AD:	This virtual destructor seems to hit a bug in the stdlib
		//		where vector delete is incorrectly called.  Workaround is
		//		to comment this out while I investigate.
		/* dtor */ virtual ~IMedium( void ) {}
	};

// Statics:
public:
	static	bool WriteLog( const CMIUtilString & vData );

// Methods:
public:
	bool	RegisterMedium( const IMedium & vrMedium );
	bool	UnregisterMedium( const IMedium & vrMedium ); 
	bool	Write( const CMIUtilString & vData, const ELogVerbosity veType );
	bool	SetEnabled( const bool vbYes );
	bool	GetEnabled( void ) const;
	
	// MI common object handling - duplicate of CMICmnBase functions, necessary for LINUX build
	// Done to stop locking on object construction init circular dependency.
	const CMIUtilString &	GetErrorDescription( void ) const;
	void					SetErrorDescription( const CMIUtilString & vrTxt ) const;
	void					ClrErrorDescription( void ) const;

// Overridden:
public:
	// From MI::ISingleton
	virtual bool	Initialize( void );
	virtual bool	Shutdown( void );

// Methods:
private:
	/* ctor */	CMICmnLog( void );
	/* ctor */	CMICmnLog( const CMICmnLog & );
	void		operator=( const CMICmnLog & );

// Overridden:
private:
	// From CMICmnBase
	/* dtor */ virtual ~CMICmnLog( void );

// Typedef:
private:
	typedef std::map< IMedium *, CMIUtilString >	MapMediumToName_t;
	typedef std::pair< IMedium *, CMIUtilString >	MapPairMediumToName_t;

// Methods:
private:
	bool	HaveMediumAlready( const IMedium & vrMedium ) const;
	bool	UnregisterMediumAll( void ); 
	
// Attributes:
private:
	bool				m_bRecursiveDive;				// True = yes recursive, false = no
	MapMediumToName_t	m_mapMediumToName;
	bool				m_bEnabled;						// True = Logger enabled for writing to mediums, false = medium not written to
	bool				m_bInitializingATM;				// True = Yes in process of initing *this logger, false = not initing
	//
	// MI common object handling - duplicate of CMICmnBase functions, necessary for LINUX build
	bool					m_bInitialized;					// True = yes successfully initialized, false = no yet or failed
	mutable CMIUtilString	m_strMILastErrorDescription;
	MIint					m_clientUsageRefCnt;			// Count of client using *this object so not shutdown() object to early
};
