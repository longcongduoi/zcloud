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
CND_CONF=Release
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/src/Clients.o \
	${OBJECTDIR}/src/Heap.o \
	${OBJECTDIR}/src/LinkedList.o \
	${OBJECTDIR}/src/Log.o \
	${OBJECTDIR}/src/MQTTAsync.o \
	${OBJECTDIR}/src/MQTTClient.o \
	${OBJECTDIR}/src/MQTTPacket.o \
	${OBJECTDIR}/src/MQTTPacketOut.o \
	${OBJECTDIR}/src/MQTTPersistence.o \
	${OBJECTDIR}/src/MQTTPersistenceDefault.o \
	${OBJECTDIR}/src/MQTTProtocolClient.o \
	${OBJECTDIR}/src/MQTTProtocolOut.o \
	${OBJECTDIR}/src/MQTTVersion.o \
	${OBJECTDIR}/src/Messages.o \
	${OBJECTDIR}/src/SSLSocket.o \
	${OBJECTDIR}/src/Socket.o \
	${OBJECTDIR}/src/SocketBuffer.o \
	${OBJECTDIR}/src/StackTrace.o \
	${OBJECTDIR}/src/Thread.o \
	${OBJECTDIR}/src/Tree.o \
	${OBJECTDIR}/src/utf-8.o


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
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ../../src/extralibs/libpahomqttv3.a

../../src/extralibs/libpahomqttv3.a: ${OBJECTFILES}
	${MKDIR} -p ../../src/extralibs
	${RM} ../../src/extralibs/libpahomqttv3.a
	${AR} -rv ../../src/extralibs/libpahomqttv3.a ${OBJECTFILES} 
	$(RANLIB) ../../src/extralibs/libpahomqttv3.a

${OBJECTDIR}/src/Clients.o: src/Clients.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Clients.o src/Clients.c

${OBJECTDIR}/src/Heap.o: src/Heap.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Heap.o src/Heap.c

${OBJECTDIR}/src/LinkedList.o: src/LinkedList.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/LinkedList.o src/LinkedList.c

${OBJECTDIR}/src/Log.o: src/Log.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Log.o src/Log.c

${OBJECTDIR}/src/MQTTAsync.o: src/MQTTAsync.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTAsync.o src/MQTTAsync.c

${OBJECTDIR}/src/MQTTClient.o: src/MQTTClient.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTClient.o src/MQTTClient.c

${OBJECTDIR}/src/MQTTPacket.o: src/MQTTPacket.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTPacket.o src/MQTTPacket.c

${OBJECTDIR}/src/MQTTPacketOut.o: src/MQTTPacketOut.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTPacketOut.o src/MQTTPacketOut.c

${OBJECTDIR}/src/MQTTPersistence.o: src/MQTTPersistence.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTPersistence.o src/MQTTPersistence.c

${OBJECTDIR}/src/MQTTPersistenceDefault.o: src/MQTTPersistenceDefault.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTPersistenceDefault.o src/MQTTPersistenceDefault.c

${OBJECTDIR}/src/MQTTProtocolClient.o: src/MQTTProtocolClient.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTProtocolClient.o src/MQTTProtocolClient.c

${OBJECTDIR}/src/MQTTProtocolOut.o: src/MQTTProtocolOut.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTProtocolOut.o src/MQTTProtocolOut.c

${OBJECTDIR}/src/MQTTVersion.o: src/MQTTVersion.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/MQTTVersion.o src/MQTTVersion.c

${OBJECTDIR}/src/Messages.o: src/Messages.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Messages.o src/Messages.c

${OBJECTDIR}/src/SSLSocket.o: src/SSLSocket.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/SSLSocket.o src/SSLSocket.c

${OBJECTDIR}/src/Socket.o: src/Socket.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Socket.o src/Socket.c

${OBJECTDIR}/src/SocketBuffer.o: src/SocketBuffer.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/SocketBuffer.o src/SocketBuffer.c

${OBJECTDIR}/src/StackTrace.o: src/StackTrace.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/StackTrace.o src/StackTrace.c

${OBJECTDIR}/src/Thread.o: src/Thread.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Thread.o src/Thread.c

${OBJECTDIR}/src/Tree.o: src/Tree.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/Tree.o src/Tree.c

${OBJECTDIR}/src/utf-8.o: src/utf-8.c 
	${MKDIR} -p ${OBJECTDIR}/src
	${RM} "$@.d"
	$(COMPILE.c) -O2 -MMD -MP -MF "$@.d" -o ${OBJECTDIR}/src/utf-8.o src/utf-8.c

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ../../src/extralibs/libpahomqttv3.a

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
