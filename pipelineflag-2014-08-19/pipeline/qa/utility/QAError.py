
# -----------------------------------------------------------------------------

# QAError

# Description:
# ------------
# This module contains the class that handles exceptions for the QA system.

# Inherited classes:
# ------------------
# Exception - The Exception class.

# Members:
# --------
# __init__ - The constructor.
# __str__  - The string function.
# message  - The python string containing the error message.  It is copied from
# the constructor input parameter.

# Modification history:
# --------------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.

# -----------------------------------------------------------------------------

class QAError( Exception ):

# -----------------------------------------------------------------------------

# __init__

# Description:
# ------------
# This member function is the class constructor.

# Inputs:
# -------
# message - This python string contains the error message.

# Outputs:
# --------
# None.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.

# -----------------------------------------------------------------------------

	def __init__( self, message ):

		self.message = message

		return

# -----------------------------------------------------------------------------

# __str__

# Description:
# ------------
# This member function saves the error message string.

# Inputs:
# -------
# None.

# Outputs:
# --------
# None.

# Modification history:
# ---------------------
# 2011 May 20 - Nick Elias, NRAO
#               Initial version.

# -----------------------------------------------------------------------------

	def __str__( self ):

		return repr( self.message )
