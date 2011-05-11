/* servo.x */

/*
 * Copyright (C) 1998, 1999  CSIRO Australia Telescope
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version. 
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Library General Public License for more details. 
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * RPCL file for astronomical antenna servo subsystem.
 *
 * Author: David G Loone
 *
 * Status: almost final
 */

/* $Id$ */

/*
 * Intended Use:
 * ============
 *
 * This interface is designed to present an idealised antenna servo
 * system. It is intended that it will cope with both the Compact Array
 * style of antenna servo system, and the Parkes antenna servo system
 * in normal operational uses, plus others in the future. It is not
 * intended to present an interface that can be used for general
 * maintenance functions on these antennas, as these functions are
 * often highly antenna specific, and usually not of much interest to
 * the observing system anyway.
 *
 * The antenna model that this interface uses is of a drive system with
 * two axes that can be driven to any position in a specified range.
 * This pair of axes and an arbitrary number of other pairs of axes
 * have encoders that can read the current absolute position of each
 * axis.
 *
 * Nomenclature:
 * ============
 *
 * Procedure names and const names are prefixed by "SERVO_RPC__".
 * Other symbol names that exist as global symbols in the program
 * module are prefixed by "servoRPC_". Procedure names and other
 * symbols are often referred to in the documentation without these
 * prefixes (eg, reference to the TRACK_SHORT procedure really means
 * the SERVO_RPC__TRACK_SHORT procedure).
 *
 * All angles are specified in radians. All times are expressed as
 * either absolute of relative BAT. All velicities are in radians per
 * second.
 *
 * Request and Readout Coordinate Systems:
 * ======================================
 *
 * The servo system can be controlled in only one coordinate system (the
 * request coordinate system), but can supply readouts in more than one
 * coordinate system (the readout coordinate systems) simultaneously.
 *
 * All coordinates that pass through the interface are dial settings,
 * that is are allowed the full range of the servo axes. In some cases
 * this may be greater than 360 degrees. Wrap issues are handled by
 * higher levels of software.
 *
 * Coordinates that pass through the interface are not corrected for the
 * antenna pointing model (ie, the servo implementation corrects
 * coordinates that it gets from the interface for the pointing model
 * before applying them to the axes, and does the reverse for readout
 * coordinates before passing them out through the interface). This means
 * that the entire angular range of each axis is presented in the
 * interface.
 *
 * It is usually (but not necessarily) the case that the request
 * coordinate system has encoders. In this case, that readout coordinate
 * system is the first, and is called "request".
 *
 * States:
 * ======
 *
 * This interface exposes a state structure to which the servo system
 * implementation must conform (at least as seen through the interface).
 * Valid state transitions are detailed in the documentation below.
 *
 * The state structure helps to get around the inherently single threaded
 * nature of RPC. None of the procedures defined here block waiting
 * for servo system movement. All procedures return immediately after
 * their parameters have been checked (eg, that track request points are
 * monotonically increasing in time) and all required return information
 * has been gathered.
 *
 * Dead Zones:
 * ==========
 *
 * The antenna servo system coordinate space can contain any number of
 * dead zones. These are a static property of the servo system and
 * represent no-go areas of the coordinate space of the servo (not
 * necessarily expressed in the request coordinate system).
 *
 * The server communicates information about the dead zones of the
 * servo system so that higher levels of the control system can not
 * only determine the places on the sky where it should not send the
 * antenna, but can determine the amount of time before a dead zone
 * is entered (eg, the horizon).
 *
 * In this version of the interface, only one dead zone specification
 * "shape" is defined in this interface (a circle), but there is
 * provision for future expansion. For example, an irregular horizon
 * could be defined by a series of arbitrary points.
 *
 * Track Request and Track Readout Queues:
 * ======================================
 *
 * The servo implementation maintains a queue of request points called
 * the "track request queue". Each element of the queue consists of a
 * time, a request point, a maximum error for the point, and a number
 * of flags. Elements can be added to the track request queue via the
 * TRACK and TRACK_SHORT procedures. Times in the track request queue
 * must be monotonically increasing (the earliest time being at
 * the head of the queue), except for guard points.
 *
 * Another queue, called the "track readout queue", is also maintained
 * by the servo implementation. Each element of the track reply queue
 * consists of a timestamp, a set of readout points (one for each
 * readout coordinate system), and a flags word. The track readout
 * queue is accessed via the GET_TRACK procedure.
 *
 * The relationship between the track request queue and track readout
 * queue is that when the time indicated in the head element of the
 * track request queue arrives, the position of the antenna is
 * determined from the encoders, and an element is added to the track
 * reply queue. The timestamp for the element is the time from the
 * track request queue. The readout points are those obtained from
 * the encoders, and the flags are constructed according to the flag
 * descriptions.
 *
 * The exception to this sequence of events is if the FLAG_NO_REPLY
 * is specified in the track request queue element, in which case
 * the element is possibly discarded when its time comes due, in
 * accordance with the description of the FLAG_NO_REPLY flag.
 *
 * Some procedures and states clear the track request queue. This means
 * that any elements that are in the track request queue are transferred
 * to the track readout queue, which the cancelled flag set (unless the
 * FLAG_NO_REPLY flag is set, in which case the element is discarded).
 *
 * Target Point and Target Time:
 * ============================
 *
 * The "target point" and "target time" are the point and time
 * contained in the head element of the track request queue. The
 * significance of these two values is that when the servo system
 * is in the "tracking" state, the antenna is moving at a rate
 * (according to the currently selected interpolation scheme) in each
 * axis such that it will arrive at the target point exactly at the
 * target time. If it cannot make the target point at the target time
 * (because it is too far away from the current posision), then the
 * antenna is in the "slewing" state and it moves towards the target
 * point at the maximum drive rate.
 *
 * If the track request queue becomes empty, then the servo system
 * changes to the "idle" state and stops antenna movement.
 *
 * Interpolation Schemes:
 * =====================
 *
 * The servo implementation can support one or more interpolation
 * schemes. It reports the set or supported interpolation schemes from
 * a set of possible schemes, then one is slected for use. The
 * possible interpolation schemes are:
 *
 * linear:
 * ------
 * Linear interpolation in each axis.
 *
 * smooth:
 * ------
 * For each axis, calcaulate the slope at each point by fitting a
 * 4th order polynomial to the point and its neighbours two deep on
 * either side. To interpolate between any two points, fit a 3rd
 * order polynomial to the two points either side, and the
 * calculated slope at each of those points.
 *
 * Pointing Model:
 * ==============
 *
 * The pointing model is very antenna specific, and any attempt to
 * define an antenna independent pointing model would be difficult to
 * do and probably not of much long term use anyway. This interface
 * assumes that the pointing model is defined outside the normal interface
 * (e.g. via named values).
 *
 * Watchdog:
 * ========
 *
 * The server maintains a watchdog timeout value. If this timeout
 * period passes without any procedures being called (including the
 * PING procedure), then the servo system internally generates a STOW
 * procedure call. The watchdog timeout value can be set and queried
 * via the API. Its initial value is obtained externally (eg, from
 * the registry).
 *
 * Token:
 * =====
 *
 * This interface specifies for active methods (i.e. methods changing the
 * state of telescope and/or servo system) a 32-bit token to limit
 * access.
 *
 * Named Values:
 * ============
 *
 * The named values mechanism gives this interface an antenna dependent
 * component. The servo implementation maintains a list of strongly
 * typed name/value pairs, which can be retrieved and (optionally,
 * under control of the servo implementation) set via this interface.
 * The list of names is not documented in this interface, and is only
 * available at runtime.
 *
 */

/**************************************************************************
 *
 * Generic data structure definitions.
 *
 **************************************************************************/

/**************************************************************************
 * Track point request flags. Collectively, these constants define a
 * bit mask that goes with each track point request.
 */

/* Don't reply to this request. If this flag is set, a readout point is
 * not normally placed in the readout queue, unless the readout was
 * outside the maximum error for the track request point, or the
 * servo system was outside the maximum error between the previous
 * point and this one (ie, either or both of the FLAG_POS_ERROR or
 * FLAG_POS_ERROR_PAST flags is be true in the readout point). */
const SERVO_RPC__FLAG_NO_REPLY = 1;
/* This point is a guard point. Treat it as a real point for the
 * purposes of calculating the interpolation (using lookahead), but
 * when the point comes to the head of the track request queue,
 * throw it away. A point in the track readout queue is never
 * generated for a guard point. Times for guard points need not be
 * monotonic in the track request queue. That is, the point following
 * a guard point can have a time before that for the guard point. */
const SERVO_RPC__FLAG_GUARD_POINT = 2;
/* Don't actually drive the servo. If this flag is specified, then the
 * point specification in the request is ignored, and the request is
 * used simply to mark a point in time that the a readout point should
 * be generated. */
const SERVO_RPC__FLAG_NO_DRIVE = 4;
/* This point is the start of a (new) track sequence. If system was idle,
 * the telescope will move already to the position at the right speed. */
const SERVO_RPC__FLAG_FIRST_POINT = 8;
  

/**************************************************************************
 * Track point readout flags. Collectively, these constants define a
 * bit mask that returns with each track point reply.
 */

/* The servo system was not within the required maximum error at the
 * time of the point as specified in the track request point. */
const SERVO_RPC__FLAG_POS_ERROR = 1;
/* The servo system was not within the required maximum error at some
 * point between this and the previous point. */
const SERVO_RPC__FLAG_POS_ERROR_PAST = 2;
/* The track request point was cancelled. */
const SERVO_RPC__FLAG_CANCELLED = 4;

/*
 * Absolute Binary Atomic Time (BAT). Must be a 64-bit unsigned
 * integer.
 */
#if !defined(JRPCGEN)
/* wnb 981119 -- made it proper struct for now -- hyper not known in pSOS */
struct __ulonglong {unsigned int h1; unsigned int h0;};
%#ifdef AIPS_LINUX
%#define _PSOS_ 
%#endif
%#ifdef _PSOS_
typedef struct __ulonglong servoRPC_AbsTime;
%#else
typedef unsigned hyper servoRPC_AbsTime;
%#endif
#else
typedef unsigned hyper servoRPC_AbsTime;
#endif

/*
 * Relative Binary Atomic Time (RBAT). Must be a 64-bit signed
 * integer.
 */
#if !defined(JRPCGEN)
/* wnb 981119 -- made it proper struct for now -- hyper not known in pSOS */
struct __longlong {int h1; unsigned int h0;};
%#ifdef _PSOS_
typedef struct __longlong servoRPC_RelTime;
%#else
typedef hyper servoRPC_RelTime;
%#endif
#else
typedef hyper servoRPC_RelTime;
#endif

/*
 * An enumeration that can be used when we need to specify the inside
 * or outside of a shape.
 */
enum servoRPC_ShapeInclude {
  /* Inside. */
  servoRPC_inside = 1,
  /* Outside. */
  servoRPC_outside = 2
};

/*
 * A readout axis pair name. This provides a text label to distinguish
 * different antenna axis pairs. Often, they can be distinguished by
 * their coordinate system, but this takes care of the hypothetical
 * (or not?) case where an antenna has two axis pairs with the same
 * coordinate system, but seperate encoders.
 */
typedef string servoRPC_ReadoutName<>;

/*
 * Status returns. These conditions describe the servo system as a whole,
 * and can potentially be returned from just about any of the procedures.
 * Any except ok mean that the procedure has not completed properly.
 */
enum servoRPC_Status {
  /* Success. */
  servoRPC_ok = 0,
  /* Failure of unknown origin. */
  servoRPC_failure = 1,
  /* An unsupported operation was requested (the feature has not yet
   * been implemented in this servo implementation). */
  servoRPC_unsupported = 2,
  /* Authentication failed. */
  servoRPC_authFailed = 3,
  /* The request was impossible due to hardware limitations. */
  servoRPC_impossible = 4,
  /* Operation failed because of a clock system error. */
  servoRPC_clockError = 5,
  /* Operation failed because of a registry system error. */
  servoRPC_registryError = 6,
  /* Invalid arguments for the procedure. */
  servoRPC_invalidArg = 7,
  /* Invalid current state for request. The procdure would have resulted
   * in a state change that is not one of the valid state transitions
   * as documented in this file. */
  servoRPC_badState = 8,
  /* Invalid coordinates were specified. */
  servoRPC_badCoord = 9,
  /* A system hardware reboot has occurred, and possibly data have to be
   * updated. This error needs a clearReboot command before any active
   * command (apart from STOW and RESET and CLEARREBOOT and
   * SETNAMEDVALUE) can be executed.
   */
  servoRPC_systemRebooted = 10,

  /* Severe hardware error. Cannot continue. This is really bad, and
   * indicates that the servo system cannot carry out any requests,
   * including stowing the antenna. */
  servoRPC_servoHardwareError = 50,
  /* Invalid pointing model. */
  servoRPC_invalidPointParams = 51,
  /* Soft servo error. Try doing a reset. */
  servoRPC_softServoError = 52,
  /* Hard servo error. Cannot continue. */
  servoRPC_hardServoError = 53,
  /* The coordinate system specified was invalid for this servo
   * implementation. */
  servoRPC_badCoordSystem = 54,
  /* The track request queue is full. Extra points are lost, but the
   * servo system will continue processing the points that are in the
   * queue, and any new ones that are placed there. However, since it
   * is impossible to determine how many points have been lost, it is
   * probably best to stop and restart again anyway. */
  servoRPC_requestQueFull = 55,
  /* A track request point was out of time order. */
  servoRPC_outOfOrder = 56,
  /* Remote operation has been locked out (by eg a keyswitch). */
  servoRPC_remOpBlocked = 57
};

/*
 * Possible drive coordinate systems. Points are defined as a pair
 * or coordinates (servoRPC_Pair). The interpretation of the
 * coordinate pair depends on the coordinate system being used.
 */
enum servoRPC_CoordSystem {
  /* Azimuth/Elevation system. c1 is azimuth, c2 is elevation. */
  servoRPC_azEl = 1,
  /* Hour angle/Declination system. c1 is hour angle, c2 is
   * declination. */
  servoRPC_haDec = 2
};

/*
 * A single coordinate pair. The interpretation depends upon the
 * coordinate system being represented.
 */
struct servoRPC_Pair {
  /* First coordinate. */
  double c1;
  /* Second coordinate. */
  double c2;
};

/*
 * The possible interpolation schemes. Every server must implement
 * at least linear interpolation. Others are optional.
 */
enum servoRPC_InterpScheme {
  /* Linear in each axis. */
  servoRPC_linear = 1,
  /* Smooth. */
  servoRPC_smooth = 2
};

/*
 * Servo system states. These represent the current state of the
 * servo system machinery. From the point of view of the interface,
 * none of them represent an error condition, however some states may
 * require special recovery procedures to be implemented (eg, resetting).
 *
 * The following descriptions describe the possible state transitions.
 * All the valid state change procedures are listed, followed in
 * parentheses by the condition (or procedure) that triggers the state
 * change. Commands that are not listed for a given state are invalid
 * for that state, and return the badState status.
 *
 * In the descriptions below, the following describes the internal
 *  events:
 *  "successful stow": The antenna has successfully stowed.
 *  "unsuccessful stow": The antenna could not stow for some reason.
 *  "successful unstow": The antenna has successfully unstowed and is
 *    at some valid position.
 *  "unsuccessful unstow": The antenna could not unstow for some reaason.
 *  "successful park": The antenna has successfully parked.
 *  "drive problem detected": A servo problem has been detected that
 *    prevents further movement.
 *  "gone into limits": The antenna was driven into the software limits.
 *  "drive done": The antenna reached the target position.
 *  "antenna stopped": The antenna has stopped moving.
 *  "request queue not empty": An element has appeared at the head of
 *    the track request queue.
 *  "request queue empty": The last element was removed from the
 *    track request queue.
 *  "target point within range": The target point has come within the
 *    acquisition range of the antenna, that is, the servo system
 *    expects to get to the position by the nominated time.
 *  "target point out of range": The servo system does not expect to
 *    get to the target point by the nominated time.
 */
enum servoRPC_State {
  /*
   * Cannot determine the current state for some reason. Anything
   * goes from this state.
   */
  servoRPC_stateUnknown = 1,

  /*
   * Stowed. This represents the most safe state that the servo system
   * can be in. The servo system can only be removed from this state
   * by calling the UNSTOW procedure.
   *
   * Generally, stowed is the only state in which the antenna can
   * safely be left unattended.
   *
   * Transitions:
   *  -> stowed (STOW)
   *  -> unstowing (UNSTOW)
   *  -> stowed (HALT)
   */
  servoRPC_stowed = 2,

  /*
   * Stowing. The servo system is in the process of going to the stow
   * position. The process of stowing will only complete when the
   * servo system has successfully stowed (indicated by the
   * servoRPC_stowed state), or there is a stow error indicated by the
   * servoRPC_stowError state), or HALT or UNSTOW are requested.
   *
   * If a drive problem is detected, a stowing servo system will do its
   * best to reset the drives and continue stowing. This is different
   * from other states, where a drive problem is signalled by going
   * into the driveError state, and waiting for the RESET procedure
   * to be called. Thus, a stowing servo system does EVERYTHING IT CAN to
   * complete the stow. If all fails, it goes to the stowError state.
   *
   * If UNSTOW or HALT are requested during a stow, the stow stops and
   * goes to the unstowing state. Depending upon the stow procedure,
   * the servo implementation may simply go from there straight to the
   * idle state (in the case of a trivial stow procedure), or may have
   * to backtrack through whatever part of the stow procedure has been
   * performed so far (in the case of a complicated stow procedure).
   *
   * The GetInfoOut.estTime field contains the current estimate of the
   * time remaining before the servo system is stowed.
   *
   * Transitions:
   *  -> stowed (successful stow)
   *  -> stowError (unsuccessful stow)
   *  -> stowing (STOW)
   *  -> unstowing (UNSTOW)
   *  -> unstowing (HALT)
   */
  servoRPC_stowing = 3,

  /*
   * Unstowing. Some antennas have a complicated and/or lengthy
   * unstowing procedure.
   *
   * The GetInfoOut.estTime field contains the current estimate of the
   * time remaining before the servo system is fully unstowed.
   *
   * Transitions:
   *  -> idle (successful unstow)
   *  -> stowError (unsuccessful unstow)
   *  -> stowing (STOW)
   */
  servoRPC_unstowing = 4,

  /*
   * Stow error. Indicates that a stow or unstow was unsuccessful.
   * Operator intervention may be urgently required.
   *
   * Transitions:
   *  -> stowing (STOW)
   *  -> stowing (RESET)
   *  -> stowError (HALT)
   */
  servoRPC_stowError = 5,

  /*
   * Parked. This represents a state where the servo system is at rest
   * and the antenna is reasonably safe, but still attended. Exit the
   * parked state by any procedure that will drive the servo system.
   *
   * Transitions:
   *  -> slewing (request queue not empty)
   *  -> stowing (STOW)
   *  -> parked (PARK)
   *  -> resetting (RESET)
   *  -> parked (HALT)
   *  -> goto (GOTO)
   */
  servoRPC_parked = 6,

  /*
   * Parking. Indicates that the servo system is going to the parked
   * position.
   *
   * The GetInfoOut.estTime field contains the current estimate of the
   * time remaining before the servo system is fully parked.
   *
   * Transitions:
   *  -> parked (successful park)
   *  -> driveError (drive problem detected)
   *  -> stowing (STOW)
   *  -> parking (PARK)
   *  -> stopping (HALT)
   */
  servoRPC_parking = 7,

  /*
   * Stopping. The servo system is in the process of slowing down
   * to enter the idle state.
   *
   * Transitions:
   *  -> driveError (drive problem detected)
   *  -> slewing (request queue not empty)
   *  -> idle (antenna stopped)
   *  -> stowing (STOW)
   *  -> parking (PARK)
   *  -> resetting (RESET)
   *  -> stopping (HALT)
   *  -> goto (GOTO)
   */
  servoRPC_stopping = 8,

  /*
   * Idle. The servo sytsem does not currently have an assignment, but is
   * not in any of the "safe" states.
   *
   * Transitions:
   *  -> driveError (drive problem detected)
   *  -> slewing (request queue not empty)
   *  -> stowing (STOW)
   *  -> parking (PARK)
   *  -> resetting (RESET)
   *  -> idle (HALT)
   *  -> goto (GOTO)
   */
  servoRPC_idle = 9,

  /*
   * Goto. The servo system is moving to get to a position in
   * response to a GOTO procedure call. When it gets to the position
   * it will go to the idle state.
   *
   * The GetInfoOut.estTime field contains the current estimate of the
   * time remaining before the servo system will reach the requested
   * position.
   *
   * The GetInfoOut.target field contains the target position for the
   * goto.
   *
   * Transitions:
   *  -> inLimits (gone into limits)
   *  -> driveError (drive problem detected)
   *  -> idle (drive done)
   *  -> stowing (STOW)
   *  -> stopping (HALT)
   *  -> goto (GOTO)
   */
  servoRPC_goto = 10,

  /*
   * Slewing. The servo system is moving under control of the request
   * queue, and the target point is not within range. Track readout
   * points will contain the actual position of the antenna.
   *
   * Transitions:
   *  -> inLimits (gone into limits)
   *  -> driveError (drive problem detected)
   *  -> stopping (request queue empty)
   *  -> tracking (target point within range)
   *  -> goto (STOW)
   *  -> stopping (HALT)
   */
  servoRPC_slewing = 11,

  /*
   * Tracking. The servo system is moving under control of the request
   * queue, and the target point is within range. This does not
   * necessarily mean that the antenna is "on source". For that
   * information the readout points should be examined.
   *
   * The GetInfoOut.estTime field contains the time of the element at
   * the head of the track request queue.
   *
   * The GetInfoOut.target field contains the position in the element
   * at the head of the track request queue.
   *
   * Transitions:
   *  -> inLimits (gone into limits)
   *  -> driveError (drive problem detected)
   *  -> stopping (request queue empty)
   *  -> slewing (target point out of range)
   *  -> goto (STOW)
   *  -> stopping (HALT)
   */
  servoRPC_tracking = 12,

  /*
   * In limits. Can only get out of limits by stowing or invoking
   * the exit limits procedure.
   *
   * When this state is entered, the track request queue is cleared.
   *
   * Transitions:
   *  -> driveError (drive problem detected)
   *  -> goto (STOW)
   *  -> goto (EXIT_LIMITS)
   */
  servoRPC_inLimits = 13,

  /*
   * A drive error has been encountered.
   *
   * When this state is entered, the track request queue is cleared.
   *
   * Transitions:
   *  -> stowing (STOW)
   *  -> resetting (RESET)
   */
  servoRPC_driveError = 14,

  /*
   * Servo drive system is in the process of resetting.
   *
   * The GetInfoOut.estTime field contains the current estimate of
   * the time remaining before the servo system will be fully reset.
   *
   * Transitions:
   *  -> parked (drives reset and antenna parked)
   *  -> inLimits (drives reset and antenna in limits)
   *  -> idle (drives reset and antenna ok)
   *  -> driveError (drive problem detected)
   *  -> stowing (STOW)
   *  -> resetting (RESET)
   */
  servoRPC_resetting = 15,

  /*
   * Number of transitions specified
   *
   */
   N_servoRPC_State
};

/*
 * Dead zone reasons. This enumerates the reasons for dead zones that
 * an servo system can have. They are mainly differentiated for reporting
 * reasons (ie, you have to keep away from them all, but report that a
 * a particular point is invalid for different reasons).
 */
enum servoRPC_DeadZoneReasons {
  /* Horizon. */
  servoRPC_horizon = 1,
  /* Pole. This indicates a region where the srevo system cannot track
   * properly because of a pole of a coordinate system. */
  servoRPC_pole = 2
};

/*
 * Dead zone shapes. This enumerates the ways we have of representing a
 * dead zone.
 */
enum servoRPC_ShapeSpec {
  /* Circle around a coordinate pair (in any of the valid servo system
   * coordinate systems). */
  servoRPC_circle = 1
};

/*
 * The representation of a circle.
 */
struct servoRPC_Circle {
  /* The coordinate system that the centre of the circle is
   * described in. */
  servoRPC_CoordSystem coordSystem;
  /* The centre of the circle (in the coordinate system indicated
   * by coordSystem). */
  servoRPC_Pair centre;
  /* The radius subtended by the circle (in radians). */
  double radius;
  /* Whether no-go area is inside or outside the circle. Inside the
   * circle includes the point defined by centre. */
  servoRPC_ShapeInclude inOut;
};

/*
 * A single dead zone specification.
 */
union servoRPC_DeadZoneSpec switch (servoRPC_ShapeSpec shape) {
case servoRPC_circle:
  servoRPC_Circle circle;
};

/*
  A single dead zone.
 */
struct servoRPC_DeadZone {
  /* The dead zone reason. */
  servoRPC_DeadZoneReasons reason;
  /* The dead zone specification. */
  servoRPC_DeadZoneSpec zone;
};

/*
 * A single track point request element.
 */
struct servoRPC_TrackPointRequest {
  /* Requested time for the point. */
  servoRPC_AbsTime time;
  /* Requested position for the point. */
  servoRPC_Pair pos;
  /* Maximum allowed error for the point. This number is maximum
   * angular error (in radians) that the servo system can be away from
   * the requested position in any direction. If the servo system is
   * outside the range when the time arrives, then it is flagged in the
   * readout queue. */
  double maxError;
  /* Flags. */
  unsigned int flags;
};

/*
 * A single track point short request element.
 */
struct servoRPC_TrackPointShortRequest {
  /* Requested position for the point. */
  servoRPC_Pair pos;
  /* Flags. */
  unsigned int flags;
};

/*
 * A single track point readout element.
 */
struct servoRPC_TrackPointReadout {
  /* The requested time for the point. These should exactly match the
   * requested times, and should be used to match requests points with
   * corresponding readout points. */
  servoRPC_AbsTime timeTag;
  /* The actual time that the encoders were read. */
  servoRPC_AbsTime timeStamp;
  /* A status for the point. */
  servoRPC_Status status;
  /* The position that the servo system was at the time. One of these for
   * each readout coordinate system available on this servo system, and
   * in the same order that they were given in the static antenna info. */
  servoRPC_Pair pos<>;
  /* Flags. */
  unsigned int flags;
};

/*
 * A single date/time specification
 */
struct servoRPC_DateTime {
  /* The requested year (as ccyy), mon, day, hr, min, sec, tick 
   * (in processor units)
   */
  unsigned int dt<7>;
};

/*
 * This enum specifies the different data types that can be recognised.
 */
enum servoRPC_DataType {
  /* Bool */
  servoRPC_DTbool = 0,
  /* Int */
  servoRPC_DTint = 1,
  /* Double */
  servoRPC_DTdouble = 2,
  /* AbsTime */
  servoRPC_DTabsTime = 3,
  /* RelTime */
  servoRPC_DTrelTime = 4,
  /* Coordinate pair */
  servoRPC_DTpair = 5,
  /* String */
  servoRPC_DTstring = 6,
  /* Angle (as Double in rad) */
  servoRPC_DTangle = 7,
  /* Time (as Double) */
  servoRPC_DTtime = 8,
  /* Number of entries */
  N_servoRPC_DataType
};

/* 
 * A structure for a data name 
 */
struct servoRPC_DataName {
  /* The type of the data */
  servoRPC_DataType type;
  /* Is data writable? */
  bool writable;
  /* The name of the data (only max 15 used) */
  string name<16>;
  /* The number of elements of data */
  int len;
};

/*
 * A data value
 */
union servoRPC_DataValue switch (servoRPC_DataType type) {
case servoRPC_DTbool:
  bool DATbool;
case servoRPC_DTint:
  int DATint;
case servoRPC_DTdouble:
  double DATdouble;
case servoRPC_DTabsTime:
  servoRPC_AbsTime DATabsTime;
case servoRPC_DTrelTime:
  servoRPC_RelTime DATrelTime;
case servoRPC_DTpair:
  servoRPC_Pair DATpair;
case servoRPC_DTstring:
  string DATstring<>;
case servoRPC_DTangle:
  double DATangle;
case servoRPC_DTtime:
  double DATtime;
};

/* 
 * A structure for a data name/value pair 
 */
struct servoRPC_DataNameValue {
  /* The name. */
  servoRPC_DataName name;
  /* The value. Values come as arrays. The union discriminator is
   * guaranteed to be the same for each element of the array. */
  servoRPC_DataValue value<>;
};

/* 
 * Data for a single short info data point.
 */
struct servoRPC_ShortInfoValue {
  /* The value. Values come as arrays. The union discriminator is
   * guaranteed to be the same for each element of the array. */
  servoRPC_DataValue value<>;
};

/*
 * A single named value name.
 */
typedef string servoRPC_Name<>;

/*
 * A single short info readout point.
 */
struct servoRPC_ShortInfoReadout {
  /* The timestamp of the reading. */
  servoRPC_AbsTime timeStamp;
  /* The array of values. One element of this array for (and in the same
   * order as) each name returned by the GET_SHORTINFO_NAMES procedure.
   * For the sake of brevity, this doesn't include the names. */
  servoRPC_ShortInfoValue info<>;
};

/**************************************************************************
 *
 * Procedure specific data structure definitions.
 *
 **************************************************************************/

/*
 * Data required by the AUTHENTICATE procedure.
 */
typedef opaque servoRPC_AuthenticateIn<>;

/*
 * Data returned by the GET_WD_TIMEOUT procedure.
 */
struct servoRPC_GetWDTimeoutOut {
  /* Status return. */
  servoRPC_Status status;
  /* Current watchdog timer value. */
  servoRPC_RelTime timeout;
};

/*
 * Data returned by the GET_STATIC_INFO procedure. This represents all
 * the non-changing information about the nature of the servo system.
 */
struct servoRPC_GetStaticInfoOut {
  /* Status return. */
  servoRPC_Status status;
  /* A descriptive name for the antenna (ie, the servo system
   * implementaiton). */
  string antennaName<>;
  /* The version number of the antenna (ie, the servo system
   * implementation). */
  string antennaVersion<>;
  /* The (site short name of the) site of the antenna. */
  string antennaSite<2>;
  /* The device instance name of the antenna. This field should contain
   * a string that is constructed in such a way that it is unique
   * across the entire observatory control system. */
  string deviceInstanceName<>;
  /* The request coordinate system. There is only one request
   * coordinate system. */
  servoRPC_CoordSystem request;
  /* The readout coordinate systems. There can be one or more
   * readout coordinate systems (ie, must be at least one). If the request
   * axes have encoders, then the first element of this array is
   * the same as the request coordinate system. The servo implementation
   * documentation must describe the significance of each other
   * set of axes. */
  servoRPC_CoordSystem readouts<>;
  /* The names of the readout coordinate systems (eg, "me"). This
   * can be used to distinguish two sets of readout axes which happen
   * to have the same coordinate sysetm. This array must have one
   * element for each readout coordinate system. If the request
   * axes have encoders, then the first element of this array must
   * be "request". */
  servoRPC_ReadoutName readoutNames<>;
  /* The available interpolation schemes. */
  servoRPC_InterpScheme interpSchemes<>;
  /* The dead zones of the servo system. Can be zero or more of
   * these. */
  servoRPC_DeadZone deadZones<>;
  /* Minimum allowed value for each request coordinate. This represents
   * minimum valid dial value for each axis. Any point that contains
   * values larger than these generates a badCoord status. This
   * effectively represents a soft limit of each coordinate. */
  servoRPC_Pair min;
  /* Maximum allowed value for each request coordinate. This represents
   * the maximum valid dial value for each axis. Any point that contains
   * values smaller than these generates a badCoord status. This
   * effectively represents a soft limit of each coordinate. */
  servoRPC_Pair max;
  /* The length (number of points) of the track request queue. */
  unsigned int requestQueLen;
  /* The length (number of points) of the track readout queue. */
  unsigned int readoutQueLen;
};

/*
 * Data returned by the GET_INFO procedure. This represents all the
 * dynamic servo system information (ie, what the servo system is
 * doing "now").
 */
struct servoRPC_GetInfoOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Current state. */
  servoRPC_State state;
  /* Current reboot state (True if machine recently rebooted). Can only be
   * reset with the ClearReboot command. */
  bool rebooted;
  /* Current interpolation scheme. */
  servoRPC_InterpScheme interpScheme;
  /* Current measured position in each readout coordinate system. */
  servoRPC_Pair pos<>;
  /* Current requested position the request coordinate system. */
  servoRPC_Pair reqPos;
  /* Current measured rate in each readout coordinate system. */
  servoRPC_Pair rate<>;
  /* Current requested rate in the request coordinate system. */
  servoRPC_Pair reqRate;
  /* The point that the antenna is moving to (in the request coordinate
   * system). The interpretation of this will vary depending on the
   * state of the servo system, and the interpretation is described in
   * the state descriptions. */
  servoRPC_Pair target;
  /* Estimated time at which the operation will be complete. The
   * interpretation of this will vary depending on the state of
   * the servo system, and the interpretation is describe in the
   * state descriptions. */
  servoRPC_AbsTime estTime;
  /* Space remaining (number of unoccupied positions) in the track
   * request queue. */
  unsigned int requestQueSpace;
  /* Space remaining (number of unoccupied positions) in the track
   * readout queue. */
  unsigned int readoutQueSpace;
  /* Is remote operation blocked? */
  bool remOpBlocked;
  /* Drive error. One boolean for each request axis. Set to true if
   * there is an unrecoverable error on that drive. */
  bool driveError<2>;
};

/*
 * Data needed by the STOW procedure
 */
struct servoRPC_StowIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the STOW procedure.
 */
struct servoRPC_StowOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Estimated time at which the stow will complete. This time will
   * be continually updated in GetInfoOut.estTime element returned
   * by the GET_INFO procedure. */
  servoRPC_AbsTime estTime;
  /* Maximum time to complete. The stow MUST complete by this time,
   * or an alarm is raised. */
  servoRPC_AbsTime maxTime;
};

/*
 * Data needed by the UNSTOW procedure
 */
struct servoRPC_UnstowIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the UNSTOW procedure.
 */
struct servoRPC_UnstowOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Estimated time at which the unstow will complete. This time will
   * be continually updated in GetInfoOut.estTime element returned
   * by the GET_INFO procedure. */
  servoRPC_AbsTime estTime;
  /* Maximum time to complete. The unstow MUST complete by this time,
   * or an alarm is raised. */
  servoRPC_AbsTime maxTime;
};

/*
 * Data needed by the PARK procedure
 */
struct servoRPC_ParkIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the PARK procedure.
 */
struct servoRPC_ParkOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Estimated time at which the park will complete. This time will
   * be continually updated in GetInfoOut.estTime element returned
   * by the GET_INFO procedure. */
  servoRPC_AbsTime estTime;
  /* Maximum time to complete. The park MUST complete by this time,
   * or an alarm is raised. */
  servoRPC_AbsTime maxTime;
};

/*
 * Data needed by the RESET procedure
 */
struct servoRPC_DriveResetIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the RESET procedure.
 */
struct servoRPC_DriveResetOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Estimated time at which the drive reset will complete. This
   * time will be continually updated in GetInfoOut.estTime element
   * returned by the GET_INFO procedure. */
  servoRPC_AbsTime estTime;
  /* Maximum time to complete. The drive reset MUST complete by
   * this time, or an alarm is raised. */
  servoRPC_AbsTime maxTime;
};

/*
 * Data needed by the EXIT_LIMITS procedure
 */
struct servoRPC_ExitLimitsIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the EXIT_LIMITS procedure.
 */
struct servoRPC_ExitLimitsOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Estimated time at which the exit limits will complete. This
   * time will be continually updated in GetInfoOut.estTime
   * element returned by the GET_INFO procedure. */
  servoRPC_AbsTime estTime;
  /* Maximum time to complete. The exit limits MUST complete by
   * this time, or an alarm is raised. */
  servoRPC_AbsTime maxTime;
};

/*
 * Data needed by the HALT procedure
 */
struct servoRPC_HaltIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the HALT procedure.
 */
struct servoRPC_HaltOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Estimated time at which the halt will complete. This time will
   * be continually updated in GetInfoOut.estTime element returned
   * by the GET_INFO procedure. */
  servoRPC_AbsTime estTime;
  /* Maximum time to complete. The halt MUST complete by
   * this time, or an alarm is raised. */
  servoRPC_AbsTime maxTime;
};

/*
 * Data required by the GOTO procedure.
 */
struct servoRPC_GotoIn {
  /* Security token */
  unsigned int token;
  servoRPC_Pair pos;
};

/*
 * Data produced by the GOTO procedure.
 */
struct servoRPC_GotoOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Estimated time at which the goto will complete. This time will
   * be continually updated in GetInfoOut.estTime element returned
   * by the GET_INFO procedure. */
  servoRPC_AbsTime estTime;
  /* Maximum time to complete. The goto MUST complete by
   * this time, or an alarm is raised. */
  servoRPC_AbsTime maxTime;
};

/*
 * Data required by the TRACK procedure.
 *
 * This structure explicitly matches coordinate pairs to times. If
 * the time points are equally spaced, use the TRACK_SHORT
 * procedure.
 */
struct servoRPC_TrackIn {
  /* Security token */
  unsigned int token;
  /* An arbitrary number of track request coordinates. */
  servoRPC_TrackPointRequest points<>;
};

/*
 * Data required by the TRACK_SHORT procedure.
 *
 * This structure contains a start time, a time interval and a number
 * of coordinate pairs. The coordinate pairs are matched to times such
 * that points[i] is matched to time
 *    startTime + interval * i
 */
struct servoRPC_TrackShortIn {
  /* Security token */
  unsigned int token;
  /* The start time for the track request points. */
  servoRPC_AbsTime startTime;
  /* The time interval between points. */
  servoRPC_RelTime interval;
  /* Maximum allowed error for all the points. This number is maximum
   * angular error (in radians) that the servo system can be away from the
   * requested position in any direction. If the servo system is outside
   * the range when the time arrives, then it is flagged in the
   * readout queue. */
  double maxError;
  /* Array of short track point requests. */
  servoRPC_TrackPointShortRequest points<>;
};

/*
 * Data produced by the TRACK and TRACK_SHORT procedures.
 */
struct servoRPC_TrackOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
};

/*
 * Data required by the GET_TRACK procedure.
 */
struct servoRPC_GetTrackIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the GET_TRACK procedure.
 */
struct servoRPC_GetTrackOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
  /* Indicates the number of points that have overflowed out of the
   * readout queue. Normally, this should be zero. If the queue has
   * overflowed, then the least recent points have been lost. */
  unsigned int numOverflowed;
  /* List of times and readout positions. */
  servoRPC_TrackPointReadout points<>;
};

/*
 * Data needed by the CLEAR_TRACK_READOUT_QUE procedure
 */
struct servoRPC_ClearTrackReadoutQueIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data needed by the CLEARREBOOT procedure
 */
struct servoRPC_ClearRebootIn {
  /* Security token */
  unsigned int token;
};

/*
 * Data produced by the CLEARREBOOT procedure.
 */
struct servoRPC_ClearRebootOut {
  /* Timestamp. */
  servoRPC_AbsTime timeStamp;
  /* Status return. */
  servoRPC_Status status;
};

/*
 * Data required by the ESTIMATE_TIME procedure.
 */
struct servoRPC_EstimateTimeIn {
  /* The first position. Specified in the request coordinate
   * system. */
  servoRPC_Pair start;
  /* The second position. Specified in the request coordinate
   * system. */
  servoRPC_Pair end;
};

/*
 * Data produced by the ESTIMATE_TIME procedure.
 */
struct servoRPC_EstimateTimeOut {
  /* Status return. */
  servoRPC_Status status;
  /* The minimum time estimate. This will represent the time that
   * the antenna will take at maximum drive rate, as it would be if
   * it has a "running start" and a "running stop". */
  servoRPC_RelTime minDelay;
  /* The maximum time estimate. This will represent the time taken
   * for the antenna to traverse the distance from a standing start
   * to a standing stop. */
  servoRPC_RelTime maxDelay;
};

/*
 * Data required by the GET_BEAMWIDTH procedure.
 */
struct servoRPC_GetBeamwidthIn {
  /* The requested frequency (in Hz). */
  double frequency;
};

/*
 * Data produced by the GET_BEAMWIDTH procedure.
 */
struct servoRPC_GetBeamwidthOut {
  /* Status return. */
  servoRPC_Status status;
  /* The coordinate system in which the beamwidth is represented. */
  servoRPC_CoordSystem coordSystem;
  /* The beamwidth (in radians) in each axis of the specified
   * indicated system. */
  servoRPC_Pair width;
};

/*
 * Data required by the SET_DATETIME procedure.
 */
struct servoRPC_SetDateTimeIn {
  /* Security token */
  unsigned int token;
  servoRPC_DateTime dateTime;
};

/*
 * Data produced by the SET_DATETIME procedure.
 */
struct servoRPC_SetDateTimeOut {
  /* Status return. */
  servoRPC_Status status;
  /* Time as set */
  servoRPC_AbsTime timeStamp;
};

/*
 * Data required by the SET_SHORTINFO_NAMES procedure.
 */
struct servoRPC_SetShortInfoNamesIn {
  /* The security token. */
  unsigned int token;
  /* The array of names. If this array is empty (ie, the number of
   * elements is zero) then the short info name list is reset to the
   * system default. Must call GET_SHORTINFO_NAMES to find out
   * what that list is. The size of this array must not be greater
   * than the maximum size returned by the GET_SHORTINFO_NAMES
   * procedure. */
  servoRPC_Name names<>;
};

/*
 * Data produced by the SET_SHORTINFO_NAMES procedure.
 */
struct servoRPC_SetShortInfoNamesOut {
  /* Status return. Will contain
   *  - servoRPC_invalidArg:
   *    - one of the names was not available in the system.
   *    - too many names were specified. */
  servoRPC_Status status;
};

/*
 * Data required by the GET_SHORTINFO procedure.
 */
struct servoRPC_GetShortInfoIn {
  /* The security token. */
  unsigned int token;
};

/*
 * Data produced by the GET_SHORTINFO_NAMES procedure.
 */
struct servoRPC_GetShortInfoNamesOut {
  /* Status return. */
  servoRPC_Status status;
  /* The maximum number of names/values that can be specified/returned in
   * short info. */
  unsigned int maxValues;
  /* The array describing the named values that will be collected and
   * returned. This array is guaranteed not to be longer than the
   * value of maxValues. */
  servoRPC_DataName names<>;
};

/* 
 * Data produced by the GET_SHORTINFO procedure.
 */
struct servoRPC_GetShortInfoOut {
  /* Status return */
  servoRPC_Status status;
  /* Array of data points. One element of this array for each point
   * collected since last time GET_SHORTINFO was called. */
  servoRPC_ShortInfoReadout points<>;
};

/*
 * Data required by the GET_NAMEDVALUE procedure
 */
struct servoRPC_GetNamedValueIn {
  /* Data name. Only the name field of this struct is used. */
  servoRPC_DataName name;
};

/*
 * Data produced by the GET_NAMEDVALUE procedure
 */
struct servoRPC_GetNamedValueOut {
  /* Status return */
  servoRPC_Status status;
  /* Name. Should be the same as the name passed into the procedure,
   * but with the data type, number of elements and writeabale flag
   * filled in. */
  servoRPC_DataName name;
  /* Value. The union discriminator is guraranteed to be the same for
   * each element of this array. */
  servoRPC_DataValue value<>;
};

/*
 * Data produced by the GET_VALUES procedure.
 */
struct servoRPC_GetValuesOut {
  /* Status return. */
  servoRPC_Status status;
  /* Names and values. One element for each named value. */
  servoRPC_DataNameValue data<>;
};

/*
 * Data necessary for the SET_NAMEDVALUE procedure
 */
struct servoRPC_SetNamedValueIn {
  /* Security token. */
  unsigned int token;
  /* Data name. Only the name field of this struct is used. */
  servoRPC_DataName name;
  /* Data value. The union discriminator must be the same for each
   * elemen of this array, and must match the data type of the named
   * value being set. */
  servoRPC_DataValue value<>;
};

/*
 * Data produced by the SET_NAMEDVALUE procedure
 */
struct servoRPC_SetNamedValueOut {
  /* Status return */
  servoRPC_Status status;
  /* Name. Same as the value passed in, but with data type, array size
   * and writable flag filled in. */
  servoRPC_DataName name;
  /* Value. Same as the value passed in. */
  servoRPC_DataValue value<>;
};

/* 
 * Data produced by the GET_NAMELIST procedure
 */
struct servoRPC_GetNameListOut {
  /* Status return. */
  servoRPC_Status status;
  /* Name list. One element for each named value. */
  servoRPC_DataName names<>;
};

/**************************************************************************
 *
 * Procedure definitions.
 *
 **************************************************************************/

program SERVO_RPC__PROG {
  version SERVO_RPC__VER {

    /**********************************************************************
     * Connection maintenance procedures.
     *
     * These procedures are purely concerned with maintaining the
     * integrity of the network connection to the server.
     */

    /*
     * Ping the server to check that it is ok, and/or keep it alive.
     * The only non-ok status that can be returned by this procedure
     * is servoRPC_systemRebooted.
     */
    servoRPC_Status
    SERVO_RPC__PING(void) = 1;

    /*
     * Perform authentication.
     */
    servoRPC_Status
    SERVO_RPC__AUTHENTICATE(servoRPC_AuthenticateIn) = 2;

    /*
     * Set the watchdog timeout value.
     */
    servoRPC_Status
    SERVO_RPC__SET_WD_TIMEOUT(servoRPC_RelTime) = 3;

    /*
     * Get the current watchdog timeout value.
     */
    servoRPC_GetWDTimeoutOut
    SERVO_RPC__GET_WD_TIMEOUT(void) = 4;

    /**********************************************************************
     * Set and query procedures.
     *
     * These procedures are simply to set and get information in the
     * servo system.
     */

    /*
     * Get static servo system information.
     */
    servoRPC_GetStaticInfoOut
    SERVO_RPC__GET_STATIC_INFO(void) = 10;

    /*
     * Get dynamic servo system information.
     *
     * This procedure is guaranteed to be atomic with respect to the
     * servo cycle. That is, all information returned in the struct
     * is guaranteed to apply to the same iteration of the underlying
     * servo cycle.
     */
    servoRPC_GetInfoOut
    SERVO_RPC__GET_INFO(void) = 11;

    /*
     * Set the interpolation scheme.
     */
    servoRPC_Status
    SERVO_RPC__SET_INTERP_SCHEME(servoRPC_InterpScheme) = 12;

    /**********************************************************************
     * State change procedures.
     *
     * These procedures potentially affect the state of the servo
     * system. They are the only procedures that can return the
     * badState status code. The states themselves, and which
     * procedures are valid for each state, are described above.
     */

    /*
     * Stow the telescope. The telescope should do all it can to
     * honour this request above all others.
     *
     * The servo system will immediately go into the STOWING state, and
     * may remain there for some time. It MUST then go into either the
     * STOWED state, or the STOW_ERROR state.
     *
     * The track request queue is cleared.
     *
     * The return value contains both an estimated time to complete,
     * and a maximum time to complete. If the maximum time to complete
     * expires, than an alarm is raised.
     */
    servoRPC_StowOut
    SERVO_RPC__STOW(servoRPC_StowIn) = 30;

    /*
     * Unstow the telescope.
     *
     * The servo system will immediately go into the UNSTOWING state,
     * and may remain there for some time. It MUST then go into either
     * the IDLE state, or the STOW_ERROR state.
     *
     * The return value contains both an estimated time to complete,
     * and a maximum time to complete. If the maximum time to complete
     * expires, than an alarm is raised.
     */
    servoRPC_UnstowOut
    SERVO_RPC__UNSTOW(servoRPC_UnstowIn) = 31;

    /*
     * Park the telescope.
     *
     * The return value contains both an estimated time to complete,
     * and a maximum time to complete. If the maximum time to complete
     * expires, than an alarm is raised.
     */
    servoRPC_ParkOut
    SERVO_RPC__PARK(servoRPC_ParkIn) = 32;

    /*
     * Reset the drive system.
     *
     * The return value contains both an estimated time to complete,
     * and a maximum time to complete. If the maximum time to complete
     * expires, than an alarm is raised.
     */
    servoRPC_DriveResetOut
    SERVO_RPC__RESET(servoRPC_DriveResetIn) = 33;

    /*
     * Exit from limits to some valid position.
     *
     * The return value contains both an estimated time to complete,
     * and a maximum time to complete. If the maximum time to complete
     * expires, than an alarm is raised.
     */
    servoRPC_ExitLimitsOut
    SERVO_RPC__EXIT_LIMITS(servoRPC_ExitLimitsIn) = 34;

    /*
     * Stop servo system motion. 
     *
     * The track request queue is cleared.
     *
     * The return value contains both an estimated time to complete,
     * and a maximum time to complete. If the maximum time to complete
     * expires, than an alarm is raised.
     */
    servoRPC_HaltOut
    SERVO_RPC__HALT(servoRPC_HaltIn) = 35;

    /*
     * Goto a position. This procedure will work, even in the
     * absence of a timeframe.
     *
     * The return value contains both an estimated time to complete,
     * and a maximum time to complete. If the maximum time to complete
     * expires, than an alarm is raised.
     */
    servoRPC_GotoOut
    SERVO_RPC__GOTO(servoRPC_GotoIn) = 40;

    /*
     * Add points to the track request queue. Points are specified in
     * full format. The timeframe must be present for tracking to work.
     *
     * If the track request queue is empty, then calling this procedure
     * will change the antenna to the tracking state. This procedure
     * can only be called when the antenna is in the stopping, idle,
     * tracking or inLimits states.
     */
    servoRPC_TrackOut
    SERVO_RPC__TRACK(servoRPC_TrackIn) = 50;

    /*
     * Add points to the track request queue. Points are specified in
     * short format. The timeframe must be present for tracking to work.
     *
     * Except for the expansion of the short form of track requests, this
     * procedure functions identically to the TRACK procedure.
     */
    servoRPC_TrackOut
    SERVO_RPC__TRACK_SHORT(servoRPC_TrackShortIn) = 51;

    /*
     * Clear the reboot status. Reboot status is set when the servo hardware
     * has been rebooted. In that case only STOW, RESET, CLEARREBOOT and
     * SETNAMEDVALUE can be executed.
     */
    servoRPC_ClearRebootOut
    SERVO_RPC__CLEAR_REBOOT(servoRPC_ClearRebootIn) = 52;

    /**********************************************************************
     * Track readout procedures.
     *
     * These procedures manage the track readout queue.
     */

    /*
     * Get contents of the track readout queue. The current contents of
     * the track readout queue are returned. This should contain all the
     * points up to the current time.
     */
    servoRPC_GetTrackOut
    SERVO_RPC__GET_TRACK(servoRPC_GetTrackIn) = 70;

    /*
     * Clear the track readout queue.
     */
    servoRPC_Status
    SERVO_RPC__CLEAR_TRACK_READOUT_QUE(servoRPC_ClearTrackReadoutQueIn) = 71;

    /**********************************************************************
     * Utility procedures.
     *
     * These procedures are for making enquiries about the dynamic
     * capabilities of the servo system.
     */

    /*
     * Estimate time taken to travel between two arbitrary points at the
     * slew rate of the servo system.
     */
    servoRPC_EstimateTimeOut
    SERVO_RPC__ESTIMATE_TIME(servoRPC_EstimateTimeIn) = 90;

    /*
     * Return the beamwidth (half power point) of the antenna for a
     * given frequency.
     *
     * The actual beamwidth usually depends upon all sorts of things,
     * so this procedure should only be used as a last resort and
     * approximate indication of the real beamwidth. It represents
     * the servo system's "best guess" as to what the beamwidth might
     * be for a given frequency.
     */
    servoRPC_GetBeamwidthOut
    SERVO_RPC__GET_BEAMWIDTH(servoRPC_GetBeamwidthIn) = 91;

    /*
     * Synchronise the time of the server. This is only to be used if there
     * is no absolute time running, or it is wrong.
     *
     * This procedure will be removed at some future time when it is no
     * longer necessary.
     */
    servoRPC_SetDateTimeOut
    SERVO_RPC__SET_DATETIME(servoRPC_SetDateTimeIn) = 92;

    /*
     * Set the list of names returned by GET_SHORTINFO. Doing this
     * clears the short info circular buffer.
     */
    servoRPC_SetShortInfoNamesOut
    SERVO_RPC__SET_SHORTINFO_NAMES(servoRPC_SetShortInfoNamesIn) = 98;

    /*
     * Get details of the names returned by GET_SHORTINFO.
     */
    servoRPC_GetShortInfoNamesOut
    SERVO_RPC__GET_SHORTINFO_NAMES(servoRPC_GetShortInfoIn) = 99;

    /*
     * Get the currently accumulated short info. Doing this clears
     * the short info circular buffer.
     */
    servoRPC_GetShortInfoOut
    SERVO_RPC__GET_SHORTINFO(servoRPC_GetShortInfoIn) = 93;

    /*
     * Get the value of a named variable.
     */
    servoRPC_GetNamedValueOut
    SERVO_RPC__GET_NAMEDVALUE(servoRPC_GetNamedValueIn) = 94;

    /*
     * Set the value of a named variable.
     */
    servoRPC_SetNamedValueOut
    SERVO_RPC__SET_NAMEDVALUE(servoRPC_SetNamedValueIn) = 95;

    /*
     * Get a list of all named variables.
     */
    servoRPC_GetNameListOut
    SERVO_RPC__GET_NAMELIST(void) = 96;

    /*
     * Get the value of all variables.
     */
    servoRPC_GetValuesOut
    SERVO_RPC__GET_VALUES(void) = 97;

  } = 1;
} = 0x20002004;

