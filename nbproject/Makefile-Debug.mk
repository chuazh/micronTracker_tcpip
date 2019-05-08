#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=gfortran
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_DLIB_EXT=so
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/MTSimpleDemoC.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../Dist/libMTC.a ../Dist/libdc1394.a ../Dist/libraw1394.a -pthread -lvnl -lvnl_algo

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mtsimpledemoc

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mtsimpledemoc: ../Dist/libMTC.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mtsimpledemoc: ../Dist/libdc1394.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mtsimpledemoc: ../Dist/libraw1394.a

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mtsimpledemoc: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	g++ -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mtsimpledemoc ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/MTSimpleDemoC.o: MTSimpleDemoC.cc 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -I../Dist -MMD -MP -MF $@.d -o ${OBJECTDIR}/MTSimpleDemoC.o MTSimpleDemoC.cc

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/mtsimpledemoc

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
