class Debug ():

	def __init__ (self, verbose=False, debug=0):
		self.verbose = verbose
		self.debug = debug

	def debugMessage (self, str, debugLevel):
		if self.debug > 0 and debugLevel >= self.debug:
			print(str)

	def verboseMessage (self, str):
		if self.verbose:
			print(str)

	def exitMessage(self, str):
		print(str)
		exit(1)
