//
//  QtOneLineTextWidget.cpp
//  MusicPlayer
//
//  Created by Albert Zeyer on 23.01.14.
//  Copyright (c) 2014 Albert Zeyer. All rights reserved.
//

#include "QtOneLineTextWidget.hpp"
#include "QtApp.hpp"
#include "Builders.hpp"
#include "PythonHelpers.h"
#include "PyUtils.h"
#include <string>
#include <assert.h>


RegisterControl(OneLineText)

QtOneLineTextWidget::QtOneLineTextWidget(PyQtGuiObject* control) : QtBaseWidget(control) {
	
	// XXX: Move to QtBaseWidget?
	PyScopedGIL gil;
	long w = attrChain_int_default(control->attr, "width", -1);
	long h = attrChain_int_default(control->attr, "height", -1);
	if(w < 0) w = 30;
	if(h < 0) h = 22;
	control->PresetSize = Vec((int)w, (int)h);
	
	// set size: 	NSRect frame = NSMakeRect(0, 0, control->PresetSize.x, control->PresetSize.y);
	
	bool withBorder = attrChain_bool_default(control->attr, "withBorder", false);
	
	// [self setBordered:NO];
	
	/*
	if(withBorder) {
		[self setBezeled:YES];
		[self setBezelStyle:NSTextFieldRoundedBezel];
	}

	[self setDrawsBackground:NO];
	[self setEditable:NO];
	[[self cell] setUsesSingleLineMode:YES];
	[[self cell] setLineBreakMode:NSLineBreakByTruncatingTail];
	*/
	
	
}

PyObject* QtOneLineTextWidget::getTextObj() {
	PyQtGuiObject* control = getControl();
	PyObject* textObj = control ? control->subjectObject : NULL;
	Py_XINCREF(textObj);
	Py_XDECREF(control);
	return textObj;
}

void QtOneLineTextWidget::updateContent() {
	PyQtGuiObject* control = getControl();
	if(!control) return;
	
	if(control->attr && control->parent && control->parent->subjectObject) {
		PyObject* old = NULL;
		std::swap(old, control->subjectObject);
		control->subjectObject = control->attr ?
			PyObject_CallMethod(control->attr, (char*)"__get__", (char*)"(O)", control->parent->subjectObject)
			: NULL;
		Py_CLEAR(old);
	}

	std::string s = "???";
	{
		PyObject* labelContent = getTextObj();
		if(!labelContent && PyErr_Occurred()) PyErr_Print();
		if(labelContent) {
			if(!pyStr(labelContent, s)) {
				if(PyErr_Occurred()) PyErr_Print();
			}
		}
	}
	
	WeakRef selfRefCopy(*this);	
	execInMainThread_async([=]() {
		ScopedRef selfRef(selfRefCopy.scoped());
		if(selfRef) {
			auto self = dynamic_cast<QtOneLineTextWidget*>(selfRef.get());
			assert(self);
			
			self->setText(QString::fromStdString(s));
	
			PyScopedGIL gil;
			
			/*
			NSColor* color = backgroundColor(control);
			if(color) {
				[self setDrawsBackground:YES];
				[self setBackgroundColor:color];
			}
			*/
			
			//[self setTextColor:foregroundColor(control)];
			
			bool autosizeWidth = attrChain_bool_default(control->attr, "autosizeWidth", false);
			if(autosizeWidth) {
				//[self sizeToFit];
				PyObject* res = PyObject_CallMethod((PyObject*) control, (char*)"layoutLine", NULL);
				if(!res && PyErr_Occurred()) PyErr_Print();
				Py_XDECREF(res);
			}
		}
		
		Py_DECREF(control);
	});
}

