# For most cases, just set NAME and IS_MIC
# Customization
#  Don't set IS_MIC if there's no MIC version for this directory
#  Set SINK_NAME and SINK_SOLN_NAME to the sink-side .so name
# Dependencies
#  reads rootname.mk to set NAME

# Base name for source files and executable
NAME=$(shell cat rootname.mk)

# IS_MIC is 1 (typical) if there's a MIC version, 0 if not
IS_MIC=1

# The solution file is derived
NAME_SOLN=$(NAME)_solution

# Override SINK_NAME and SINK_SOLN_NAME if you want to explicitly specify
# This is the one file that has this override
#SINK_NAME=$(NAME)_mic
SINK_NAME=a4_sink_1
#SINK_SOLN_NAME=$(NAME_SOLN)_mic
SINK_SOLN_NAME=a4_sink_1
