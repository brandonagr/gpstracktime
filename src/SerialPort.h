#pragma once

// SerialPort.h : header file
//
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <string>
#include <windows.h>

typedef unsigned char BYTE;
typedef std::string CString;
typedef unsigned long DWORD;

/////////////////////////////////////////////////////////////////////////////
// CSerialPort window
//////////////////////////////////////////////////////////////////////
// SerialPort.h: implementation of the CSerialPort class.

// Written by Shibu K.V (shibukv@erdcitvm.org)
// Copyright (c) 2002
//
// To use CSerialPort, follow these steps:
//   - Copy the files SerialPort.h & SerialPort.cpp and paste it in your
//		Projects Directory.
//   - Take Project tab from your VC++ IDE,Take Add to Project->Files.Select files 
//		SerialPort.h & SerialPort.cpp and click ok
//	 -	Add the line #include "SerialPort.h" at the top of your Dialog's Header File.
//   - Create an instance of CSerialPort in your dialogs header File.Say
//		CSerialPort port;
 
// Warning: this code hasn't been subject to a heavy testing, so
// use it on your own risk. The author accepts no liability for the 
// possible damage caused by this code.
//
// Version 1.0  7 Sept 2002.

//////////////////////////////////////////////////////////////////////

class CSerialPort
{
// Construction
public:
	CSerialPort();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSerialPort)
	//}}AFX_VIRTUAL

// Implementation
public:
	void ClosePort();
	BOOL ReadByte(BYTE &resp);
	BOOL WriteByte(BYTE bybyte);
	BOOL OpenPort(CString portname);
	BOOL SetCommunicationTimeouts(DWORD ReadIntervalTimeout,DWORD ReadTotalTimeoutMultiplier,DWORD ReadTotalTimeoutConstant,DWORD WriteTotalTimeoutMultiplier,DWORD WriteTotalTimeoutConstant);
	BOOL ConfigurePort(DWORD BaudRate,BYTE ByteSize,DWORD fParity,BYTE  Parity,BYTE StopBits);
	HANDLE hComm;
	DCB      m_dcb;
	COMMTIMEOUTS m_CommTimeouts;
	BOOL     m_bPortReady;
	BOOL     bWriteRC;
	BOOL     bReadRC;
	DWORD iBytesWritten;
	DWORD iBytesRead;
	DWORD dwBytesRead;
	virtual ~CSerialPort();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSerialPort)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	//DECLARE_MESSAGE_MAP()
};

