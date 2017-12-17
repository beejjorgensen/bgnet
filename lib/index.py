#!/usr/bin/python

class IndexException(Exception):
	pass

class IndexEntry(object):
	def __init__(self, entid=None, key=None, parent=None, keynode=None):
		self.id = entid         # unique internal ID
		self.key = key          # a;b;c
		if key != None:
			self.localkey = key.split(';')[-1] # just c (from a;b;c)
		else:
			self.localkey = None
		self.pages = []         # page numbers, or whatever
		self.keynode = keynode  # xml Node

		self.seeonly = None  # string
		self.seealso = []   # list of strings

		self.parent = parent  # IndexEntry
		self.children = []    # IndexEntry list

	def __str__(self):
		if self.id == None: idstr = "NoId"
		else: idstr = self.id

		if self.key == None: key = "NoKey"
		else: key = self.key

		return "IndexEntry<%d:%s,%s>" % (id(self), idstr, key)

	def findSubEntry(self, id=None, key=None):
		try:
			if id != None:
				for c in self.children:
					if c.id == idstr: return c
			elif key != None:
				for c in self.children:
					if c.localkey == key: return c
		except:
			pass

		return None
	
	def __findSelfIndexInSiblingList(self):
		if self.parent == None:
			raise IndexException("Entry has no parent, so no siblings")

		siblings = self.parent.children
		try:
			i = siblings.index(self)
		except ValueError:
			raise IndexException("getNextSibling internal error: I don't appear to be my parent's child")

	def getFirstSibling(self):
		if self.parent == None:
			raise IndexException("Entry has no parent, so no siblings")

		return self.children[0]

	def getPrevSibling(self):
		index = self.__findSelfIndexInSiblingList()
		siblings = self.parent.children
		if index > 0:
			return siblings[index-1]

		return None # last sibling

	def getNextSibling(self):
		index = self.__findSelfIndexInSiblingList()
		siblings = self.parent.children
		if index < len(siblings)-1:
			return siblings[index+1]

		return None # last sibling

	def addPage(self, page):
		#print "adding page "+ str(page)+" to " + str(self)
		self.pages.append(page)
		#print "self.pages is now: " + str(self.pages)

	def addChild(self, e):
		if e.parent != None:
			e.parent.children.remove(self)
		e.parent = self

		# insert in order
		if self.children == [] or \
			e.localkey.lower() > self.children[-1].localkey.lower():

			# new last one, special case
			self.children.append(e)
		else:
			# normal case, insert before next largest one
			for i in xrange(len(self.children)):
				if self.children[i].localkey.lower() > e.localkey.lower():
					# insert before i
					self.children.insert(i, e)
					break

	def setParent(self, parent):
		parent.addChild(self)

	def getFullKey(self):
		return self.key

class Index(IndexEntry): # it's the root
	def __init__(self, indexInfoNode=None):
		IndexEntry.__init__(self)
		self.nextIndexId = 1

		self.keyIndex = {}  # IndexEntry by a;b;c

		if indexInfoNode != None:
			self.completeIndexTree(indexInfoNode)

	def completeIndexTree(self, indexInfoNode):
		if indexInfoNode == []: return

		entries = indexInfoNode.getElementsByTagName('indexentry')
		for e in entries:
			id = e.getAttribute('id')
			if id == '':
				sys.stderr.write("indexentry element missing id attribute\n");
				sys.exit(2)

			nodeEnt = self.findEntryByKey(id)
			if nodeEnt == None:
				nodeEnt = IndexEntry(key=id)  # make a new one 
				self.insertEntry(nodeEnt, id)
			nodeEnt.keynode = e  # save the xml node for later

			# construct any see-only nodes
			seeonly = e.getAttribute('see')
			if seeonly != '':
				seeonlyEnt = self.findEntryByKey(seeonly)
				if seeonlyEnt == None:
					seeonlyEnt = IndexEntry(key=seeonly)
					self.insertEntry(seeonlyEnt, seeonly)
				nodeEnt.seeonly = seeonly

			# construct any see-also nodes
			seealso = e.getAttribute('seealso')
			if seealso != '':
				seealso = seealso.split('|')
				for key in seealso:
					seealsoEnt = self.findEntryByKey(key)
					if seealsoEnt == None:
						seealsoEnt = IndexEntry(key=key);
						self.insertEntry(seealsoEnt, key)
				nodeEnt.seealso = seealso # array of see-also keys

	def findEntryByKey(self, key):
		try:
			return self.keyIndex[key]
		except:
			pass

		return None

	def insertEntry(self, e, key):
		if self.keyIndex.has_key(key): return  # already in tree

		path = key.split(';')
		cur = self

		for p in path[:-1]:
			subent = cur.findSubEntry(key=p)
			if subent == None:
				subent = IndexEntry(key=p)
				cur.addChild(subent)
				self.keyIndex[p] = subent;
			cur = subent

		cur.addChild(e)
		self.keyIndex[key] = e;

	def hasEntries(self):
		return self.children != []
 
	def getIndexId(self, id=None):
		baseId = "indexId434909-" # another fun prime
		if id != None:
			return baseId + id

		self.nextIndexId += 1

		return baseId + str(self.nextIndexId)
