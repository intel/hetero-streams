# For most cases, just set NAME and IS_MIC
# Customization
#  Don't set IS_MIC if there's no MIC version for this directory
#  Set SINK_NAME_x100 and SINK_SOLN_NAME_x100 suffix to the sink-side .so name
#   for x100 architecture
#  Set SINK_NAME_x200 and SINK_SOLN_NAME_x200 suffix to the sink-side .so name
#   for x200 architecture
# Dependencies
#  reads rootname.mk to set NAME

# Base name for source files and executable
NAME=$(shell cat rootname.mk)

# IS_MIC is 1 (typical) if there's a MIC version, 0 if not
#IS_MIC=1

# The solution file is derived
NAME_SOLN=$(NAME)_solution

# Override if you want to explicitly specify libraries to load in code
SINK_NAME_x100=$(NAME)_mic
SINK_SOLN_NAME_x100=$(NAME_SOLN)_mic
SINK_NAME_x200=$(NAME)_x200
SINK_SOLN_NAME_x200=$(NAME_SOLN)_x200
