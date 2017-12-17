#!/usr/bin/python2
#
# bxc.py -- base classes for doing Beej's Guide XML conversion
#
# Release 1: based on bgconv
#
# Copyright (c) 2007  Brian "Beej Jorgensen" Hall <beej@beej.us>
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#

"""bxc.py -- base classes for doing Beej's Guide XML conversion

AppContext: Information about the application needed by the converter

Converter: Subclass this to make your own converters

"""

import sys
import re
import string
import xml.dom
import xml.dom.minidom

#---------------------------------------------------------------------
class Converter:
	def __init__(self, ac):
		"""Create a new converter.

		ac -- The AppContext instance

		Basic functionality for an XML converter.  Converters passed
		into the bxc application should inherit from this class.

		"""

		self.ac = ac

	def out(self, s):
		"""Output a string to the output file.

		Generally this should be used instead of other output routines.
		Having it all in a single place eases things like multiple file
		output.  Also, this supports a stack of outputfiles.
		
		"""

		try:
			f = self.ac.outfile[-1]
			f.write(s.encode("utf-8"))
			#f.write(s.encode("iso_8859_1"))  # latin-1
			#f.write(s)

		except IndexError:
			sys.stderr.write("%s: ignoring write past close of last " \
				"file\n" % (self.ac.scriptname))
			sys.stderr.write("    \"%s\"\n" % (s))

	def subNone(self, s, altText):
		"""Return altText if s is '' or None, else return s."""

		if s == '' or s == None: return altText
		return s

	def walkPath(self, node, path):
		"""Walks a node down a path.

		node -- where to start the search
		path -- relative to thisElement, e.g. "guideinfo/title"

		"""

		nodeNames = string.split(path, "/")
		for a in nodeNames:
			if a == ".":
				continue
			elif a == "..":
				node = node.parentNode
				if node == None: return None
			else:
				node = node.getElementsByTagName(a)
				if node == []: return None
				node = node[0]

		return node

	
	def getText(self, node, recurse=True):
		"""Returns all text nodes in this node."""

		textNodes = []

		for a in node.childNodes:
			if a.nodeType == xml.dom.Node.TEXT_NODE or \
				a.nodeType == xml.dom.Node.CDATA_SECTION_NODE:

				textNodes.append(a.nodeValue)

			elif a.nodeType == xml.dom.Node.ELEMENT_NODE:
				if recurse:
					nextData = self.getText(a, True)
					textNodes.append(nextData)

		return "".join(textNodes)

	def getTextFromPath(self, thisElement, path):
		"""Get text nodes from this element down a path.

		thisElement -- where to start the search
		path -- relative to thisElement, e.g. "guideinfo/title"

		"""

		thisElement = self.walkPath(thisElement, path)
		return self.getText(thisElement)

	def getAttrFromPath(self, thisElement, attr, path):
		"""Get the specified from the node of the relative path."""
		node = self.walkPath(thisElement, path)
		return node.getAttribute(attr)
		
	def findAncestorNamed(self, thisElement, name):
		n = thisElement.parentNode
		while n != None:
			if n.nodeName == name:
				return n
			n = n.parentNode

		return None

	def __findHandler(self, node):
		"""Find a handler in the form element_x_x_x that best represents
		this node."""
		n = node
		namelist = []
		while n != None:
			namelist.insert(0, n.nodeName)
			n = n.parentNode

		# try to find a handler, specific down to general
		while len(namelist) > 0:
			handlerName = "element_%s" % ("_".join(namelist))
			handler = getattr(self, handlerName, None)
			if handler != None: return handler
			namelist.pop(0)

		return None

	def process(self, thisElement, subName=None, handlerMethod=None):
		"""Process a node, calling handler functions.
		
		subname - only process the named type of child
		handlerMethod - call this converter method to handle child nodes"""

		#sys.stderr.write("nodename = %s\n" % (thisElement.nodeName))

		for n in thisElement.childNodes:
			# normalize node names
			if n.nodeName == "#text" or \
				n.nodeName == "#cdata-section":

				nodeName = "#PCDATA"
			else:
				nodeName = n.nodeName

			if subName != None and nodeName != subName: continue

			#sys.stderr.write("	child nodename = %s\n" % (n.nodeName))

			if handlerMethod != None:
				# call specific handler
				handlerMethod(n)

			else:
				# call the element handler if we have one
				if n.nodeType == xml.dom.Node.ELEMENT_NODE:
					handlerName = "element_%s" % (nodeName)
					handler = getattr(self, handlerName, None)
					handler = self.__findHandler(n)
					if handler != None: handler(n)
					else:
						sys.stderr.write("%s: unhandled element <%s>\n" % \
							(self.ac.scriptname, nodeName))

				# call the text node handler for text nodes
				elif n.nodeType == xml.dom.Node.TEXT_NODE or \
					n.nodeType == xml.dom.Node.CDATA_SECTION_NODE:

					self.special_PCDATA(n)

				elif n.nodeType == xml.dom.Node.COMMENT_NODE:
					self.special_COMMENT(n)

				#else:
				#	sys.stdout.write("warning: unknown node type: %s\n" % (n.nodeType))


	# default PCDATA handler
	def special_PCDATA(self, thisElement):
		"""Default handler for text data."""
		s = thisElement.nodeValue
		#s = string.strip(s)
		self.out(s)

	# default comment handler
	def special_COMMENT(self, thisElement):
		"""Default handler for comment data."""
		pass

	def special_IDENTITY(self, thisElement):
		"""Do nothing handler."""
		self.process(thisElement)

#---------------------------------------------------------------------
class AppContext:
	"""Class for holding application info from the command line.
	
	Fields:

	infilename -- the input file name

	outfile -- a stack of open outfiles; all calls to Converter.out
	           will operate on the file at the top of the stack.

	dom -- the dom after the XML has been parsed

	Other fields can be added as needed by the converter module.
	"""

	def __init__(self, scriptname):
		"""Construct a new AppContext for a given scriptname."""
		self.outfile = []  # stack of output files
		self.dom = None
		self.scriptname = scriptname
		self.usage = "usage: %s [options]"
	
	def parseXMLFilename(self, filename, parser=None):
		fp = file(filename)
		self.parseXMLFile(fp, parser)
		fp.close()

	def parseXMLFile(self, fin, parser=None):
		try:
			xmlData = fin.read()
			self.dom = xml.dom.minidom.parseString(xmlData, parser)

		except IOError, e:
			self.errorExit(">>%s: %s" % (self.infilename, e.strerror), 2)

		except xml.parsers.expat.ExpatError, e:
			sys.stderr.write("%s: XML parse error\n" % (self.scriptname))
			for a in e.args:
				sys.stderr.write("	%s\n" % (a))
			sys.exit(3)

	def getOutputFileName(self):
		if self.outfile == []: return None
		return self.outfile[-1].name

	def pushOutput(self, filename):
		"""Push a new output file on the stack by name.

		filename -- name of the file to open and put on the stack (None
		            or "-" to push sys.stdout on the stack)

		"""
		if filename == None or filename == "-":
			self.outfile.append(sys.stdout)
		else:
			try:
				self.outfile.append(file(filename, "w"))
			except IOError, e:
				self.errorExit("%s: %s" % (filename, e.strerror), 2)
	
	def popOutput(self):
		"""Pop the file at the top of the output stack and close it.
		The next file down on the stack will be used for all subsequent
		calls to Converter.out()."""

		try:
			f = self.outfile.pop()
			if f != sys.stdout:
				f.close()
		except IndexError:
			self.errorExit("internal error: pop from empty output file stack\n")

	def warn(self, strErr):
		sys.stderr.write("%s: warning: %s\n" % (self.scriptname, strErr))

	def errorExit(self, strErr=None, status=1):
		"""Print usage and exit.

		strErr -- error message to print.  Usage will be printed if this is
				  None, as will the usage from converterModule.getUsage()

		status -- exit status

		"""

		if strErr == None:
			sys.stderr.write((self.usage + "\n") % (self.scriptname))

		else:
			sys.stderr.write("%s: %s\n" % (self.scriptname, strErr))

		sys.exit(status)

