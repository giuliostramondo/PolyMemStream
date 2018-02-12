/**\file */
#ifndef SLIC_DECLARATIONS_PRFStream_H
#define SLIC_DECLARATIONS_PRFStream_H
#include "MaxSLiCInterface.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define PRFStream_PCIE_ALIGNMENT (16)


/*----------------------------------------------------------------------------*/
/*---------------------------- Interface default -----------------------------*/
/*----------------------------------------------------------------------------*/




/**
 * \brief Basic static function for the interface 'default'.
 * 
 * \param [in] param_VEC_SIZE Interface Parameter "VEC_SIZE".
 * \param [in] param_copy_repeats Interface Parameter "copy_repeats".
 * \param [in] param_prfMode Interface Parameter "prfMode".
 * \param [in] instream_aStream The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes.
 * \param [in] instream_bStream The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes.
 * \param [in] instream_cStream The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes.
 * \param [out] outstream_aOutStream The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes.
 * \param [out] outstream_bOutStream The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes.
 * \param [out] outstream_cOutStream The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes.
 */
void PRFStream(
	int64_t param_VEC_SIZE,
	int64_t param_copy_repeats,
	int64_t param_prfMode,
	const int64_t *instream_aStream,
	const int64_t *instream_bStream,
	const int64_t *instream_cStream,
	int64_t *outstream_aOutStream,
	int64_t *outstream_bOutStream,
	int64_t *outstream_cOutStream);

/**
 * \brief Basic static non-blocking function for the interface 'default'.
 * 
 * Schedule to run on an engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 * 
 * 
 * \param [in] param_VEC_SIZE Interface Parameter "VEC_SIZE".
 * \param [in] param_copy_repeats Interface Parameter "copy_repeats".
 * \param [in] param_prfMode Interface Parameter "prfMode".
 * \param [in] instream_aStream The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes.
 * \param [in] instream_bStream The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes.
 * \param [in] instream_cStream The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes.
 * \param [out] outstream_aOutStream The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes.
 * \param [out] outstream_bOutStream The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes.
 * \param [out] outstream_cOutStream The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes.
 * \return A handle on the execution status, or NULL in case of error.
 */
max_run_t *PRFStream_nonblock(
	int64_t param_VEC_SIZE,
	int64_t param_copy_repeats,
	int64_t param_prfMode,
	const int64_t *instream_aStream,
	const int64_t *instream_bStream,
	const int64_t *instream_cStream,
	int64_t *outstream_aOutStream,
	int64_t *outstream_bOutStream,
	int64_t *outstream_cOutStream);

/**
 * \brief Advanced static interface, structure for the engine interface 'default'
 * 
 */
typedef struct { 
	int64_t param_VEC_SIZE; /**<  [in] Interface Parameter "VEC_SIZE". */
	int64_t param_copy_repeats; /**<  [in] Interface Parameter "copy_repeats". */
	int64_t param_prfMode; /**<  [in] Interface Parameter "prfMode". */
	const int64_t *instream_aStream; /**<  [in] The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes. */
	const int64_t *instream_bStream; /**<  [in] The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes. */
	const int64_t *instream_cStream; /**<  [in] The stream should be of size (((param_prfMode == 0) ? param_VEC_SIZE : 0) * 8) bytes. */
	int64_t *outstream_aOutStream; /**<  [out] The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes. */
	int64_t *outstream_bOutStream; /**<  [out] The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes. */
	int64_t *outstream_cOutStream; /**<  [out] The stream should be of size (((param_prfMode == 1) ? param_VEC_SIZE : 2) * 8) bytes. */
} PRFStream_actions_t;

/**
 * \brief Advanced static function for the interface 'default'.
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in,out] interface_actions Actions to be executed.
 */
void PRFStream_run(
	max_engine_t *engine,
	PRFStream_actions_t *interface_actions);

/**
 * \brief Advanced static non-blocking function for the interface 'default'.
 *
 * Schedule the actions to run on the engine and return immediately.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * 
 * \param [in] engine The engine on which the actions will be executed.
 * \param [in] interface_actions Actions to be executed.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *PRFStream_run_nonblock(
	max_engine_t *engine,
	PRFStream_actions_t *interface_actions);

/**
 * \brief Group run advanced static function for the interface 'default'.
 * 
 * \param [in] group Group to use.
 * \param [in,out] interface_actions Actions to run.
 *
 * Run the actions on the first device available in the group.
 */
void PRFStream_run_group(max_group_t *group, PRFStream_actions_t *interface_actions);

/**
 * \brief Group run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule the actions to run on the first device available in the group and return immediately.
 * The status of the run must be checked with ::max_wait. 
 * Note that use of ::max_nowait is prohibited with non-blocking running on groups:
 * see the ::max_run_group_nonblock documentation for more explanation.
 *
 * \param [in] group Group to use.
 * \param [in] interface_actions Actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *PRFStream_run_group_nonblock(max_group_t *group, PRFStream_actions_t *interface_actions);

/**
 * \brief Array run advanced static function for the interface 'default'.
 * 
 * \param [in] engarray The array of devices to use.
 * \param [in,out] interface_actions The array of actions to run.
 *
 * Run the array of actions on the array of engines.  The length of interface_actions
 * must match the size of engarray.
 */
void PRFStream_run_array(max_engarray_t *engarray, PRFStream_actions_t *interface_actions[]);

/**
 * \brief Array run advanced static non-blocking function for the interface 'default'.
 * 
 *
 * Schedule to run the array of actions on the array of engines, and return immediately.
 * The length of interface_actions must match the size of engarray.
 * The status of the run can be checked either by ::max_wait or ::max_nowait;
 * note that one of these *must* be called, so that associated memory can be released.
 *
 * \param [in] engarray The array of devices to use.
 * \param [in] interface_actions The array of actions to run.
 * \return A handle on the execution status of the actions, or NULL in case of error.
 */
max_run_t *PRFStream_run_array_nonblock(max_engarray_t *engarray, PRFStream_actions_t *interface_actions[]);

/**
 * \brief Converts a static-interface action struct into a dynamic-interface max_actions_t struct.
 *
 * Note that this is an internal utility function used by other functions in the static interface.
 *
 * \param [in] maxfile The maxfile to use.
 * \param [in] interface_actions The interface-specific actions to run.
 * \return The dynamic-interface actions to run, or NULL in case of error.
 */
max_actions_t* PRFStream_convert(max_file_t *maxfile, PRFStream_actions_t *interface_actions);

/**
 * \brief Initialise a maxfile.
 */
max_file_t* PRFStream_init(void);

/* Error handling functions */
int PRFStream_has_errors(void);
const char* PRFStream_get_errors(void);
void PRFStream_clear_errors(void);
/* Free statically allocated maxfile data */
void PRFStream_free(void);
/* returns: -1 = error running command; 0 = no error reported */
int PRFStream_simulator_start(void);
/* returns: -1 = error running command; 0 = no error reported */
int PRFStream_simulator_stop(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* SLIC_DECLARATIONS_PRFStream_H */

