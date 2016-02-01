#include <QtGui/QApplication>
#pragma once
#ifndef BREAKPADDIALOG_H
#define BREAKPADDIALOG_H
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

#include "client/windows/handler/exception_handler.h"

class breakpaddialog : public QWidget
{
public:
	bool ask(void* context, EXCEPTION_POINTERS* exinfo,
		MDRawAssertionInfo* assertion);
};

#endif



