// Copyright (c) 2023 Robin E. R. Davies
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#pragma once
#include <cassert>
#include <string>
#include "lv2/core/lv2.h"
#include "lv2/state/state.h"
#include "lv2/worker/worker.h"
#include "lv2/options/options.h"
#include "lv2/log/logger.h"
#include "lv2/atom/atom.h"
#include "lv2/atom/forge.h"
#include "lv2/urid/urid.h"
#include "lv2/patch/patch.h"
#include "lv2/units/units.h"
#include "lv2/buf-size/buf-size.h"
#include <vector>
#include <functional>
#include <concepts>


#ifndef REGISTRATION_DECLARATION
#define REGISTRATION_DECLARATION __attribute__((used))
#endif

namespace lv2c
{
	namespace lv2_plugin
	{

		class Lv2Plugin;

		enum class Lv2LogLevel
		{
			Trace = 0,
			Note = 1,
			Warning = 2,
			Error = 3,
			None = 4,
		};
		struct BufSizeOptions
		{
			static constexpr uint32_t INVALID_VALUE = (uint32_t)-1;
			uint32_t minBlockLength = INVALID_VALUE;
			uint32_t maxBlockLength = INVALID_VALUE;
			uint32_t nominalBlockLength = INVALID_VALUE;
			uint32_t sequenceSize = INVALID_VALUE;
		};

		class Lv2Plugin
		{
		private:
			friend class Lv2PluginWithState;
			Lv2Plugin(double rate, const char *bundlePath, const LV2_Feature *const *features, bool hasState);

		protected:
			Lv2Plugin(double rate, const char *bundlePath, const LV2_Feature *const *features) : Lv2Plugin(rate, bundlePath, features, false) {}

		public:
			static constexpr bool HAS_STATE = false;

			double getRate() const { return rate; }
			const std::string &GetBundlePath() const { return bundle_path; }
			static bool HasState() { return false; }
			virtual void ConnectPort(uint32_t port, void *data) = 0;
			virtual void Activate() {}
			virtual void Run(uint32_t n_samples) = 0;
			virtual void Deactivate() {}
			// Map functions.
			LV2_URID MapURI(const char *uri);
			const char *UnmapUri(LV2_URID urid);

			// Log functions
			void LogError(const char *fmt, ...);
			void LogWarning(const char *fmt, ...);
			void LogNote(const char *fmt, ...);
			void LogTrace(const char *fmt, ...);

			void LogError(const std::string&msg) { LogError("%s",msg.c_str()); 	};

		public:
			virtual ~Lv2Plugin() {}

			static void SetLogLevel(Lv2LogLevel level)
			{
				Lv2Plugin::logLevel = level;
			}

			friend class Lv2Plugin_Callbacks;

		protected:
			LV2_URID_Map *map = nullptr;
			LV2_URID_Unmap *unmap = nullptr;
			LV2_Atom_Forge outputForge;

		protected:
			template <typename T>
			static const T *GetFeature(const LV2_Feature *const *features, const char *featureUri);

			// State extension callbacks.
			virtual LV2_State_Status
			OnRestoreLv2State(
				LV2_State_Retrieve_Function retrieve,
				LV2_State_Handle handle,
				uint32_t flags,
				const LV2_Feature *const *features)
			{
				if (!hasState)
				{
					return LV2_STATE_ERR_NO_FEATURE;
				}
				return LV2_State_Status::LV2_STATE_SUCCESS;
			}
			virtual LV2_State_Status
			OnSaveLv2State(
				LV2_State_Store_Function store,
				LV2_State_Handle handle,
				uint32_t flags,
				const LV2_Feature *const *features)
			{
				if (!hasState)
				{
					return LV2_STATE_ERR_NO_FEATURE;
				}
				return LV2_State_Status::LV2_STATE_SUCCESS;
			}

			void HandleEvents(LV2_Atom_Sequence *controlInput);
			void BeginAtomOutput(LV2_Atom_Sequence *controlOutput);

			void SetAtomPortBuffers(LV2_Atom_Sequence *controlInput,LV2_Atom_Sequence *controlOutput)
			{
				this->controlInput = controlInput;
				this->controlOutput = controlOutput;
			}

			virtual bool OnPatchPathSet(LV2_URID propertyUrid,const char*value) {
				return false;
			}

			virtual void OnPatchSet(LV2_URID propertyUrid, const LV2_Atom *value)
			{
				if (value->type == this->urids.atom__Path || value->type == this->urids.atom__String)
				{
					typedef struct {
						LV2_Atom atom; /**< Atom header. */
						const char body[1];
					} LV2_Atom_String_X;

					const char*strValue = ((LV2_Atom_String_X*)(void*)value)->body;
					if (OnPatchPathSet(propertyUrid,strValue))
					{
						PutPatchPropertyPath(0,propertyUrid,strValue);
					}
				}
			}

			void AddPatchProperty(LV2_URID propertyUrid)
			{
				this->patchPropertyUrids.push_back(propertyUrid);
			}

			virtual const char* OnGetPatchPropertyValue(LV2_URID propertyUrid)
			{
				return nullptr;
			}
			virtual void OnPatchGet(LV2_URID propertyUrid)
			{

				const char*result = OnGetPatchPropertyValue(propertyUrid);
				if (result)
				{
					PutPatchPropertyPath(0,propertyUrid,result);
				}
			}
			virtual void OnPatchGetAll()
			{
			}

		protected:
			const BufSizeOptions &GetBuffSizeOptions() const { return bufSizeOptions; }

			void PutPatchPropertyString(int64_t frameTime, LV2_URID propertyUrid, const char *value);
			void PutPatchPropertyPath(int64_t frameTime, LV2_URID propertyUrid, const char *value);
			void PutPatchPropertyUri(int64_t frameTime, LV2_URID propertyUrid, const char *value);

			void PutPatchProperty(int64_t frameTime, LV2_URID propertyUrid, bool value);
			void PutPatchProperty(int64_t frameTime, LV2_URID propertyUrid, float value);
			void PutPatchProperty(int64_t frameTime, LV2_URID propertyUrid, size_t count, const float *values);
			void PutPatchProperty(int64_t frameTime, LV2_URID propertyUrid, double value);
			void PutPatchProperty(int64_t frameTime, LV2_URID propertyUrid, int32_t value);
			void PutPatchProperty(int64_t frameTime, LV2_URID propertyUrid, int64_t value);

			void PutStateChanged(int64_t frameTime);
			// Schedule extension callbacks.

		protected:
			/// @brief Implements execution of background tasks on the lv2 worker thread.
			/// Use of WorkerAction is somewhat complicated because memory allocations are forbidden on the Audio thread.
			/// WorkerAction implementations should be declared as members of the main LV2 plugin, and should not be
			/// dynamically allocated. Generally, there should not be more than one outstanding request, so the owning
			/// plugin should manage its state so that only one request is outstanding at any given time.
			///
			/// Call @ref Request() to request an operation on the background thread. The virtual method @ref OnWork() will
			/// be called on the LV2 host's worker thread, and after it completes, @Ref OnComplete will be called on the audio
			/// thread. If the current LV2 host does not support schedule requests, the operations will be ececuted synchronously
			/// on the audio thread (almost defintely causing an audio underrun)/

			class WorkerAction
			{
			private:
				WorkerAction *pThis = nullptr;
				Lv2Plugin *pPlugin;

			protected:
				WorkerAction(Lv2Plugin *pPlugin)
				{
					pThis = this;
					this->pPlugin = pPlugin;
				}
				virtual ~WorkerAction()
				{
				}

			public:
				///@brief Request execution on the LV2 Hosts' background thread.
				///
				/// Must be called from the audo thread.
				void Request();

			protected:
				virtual void OnWork() = 0;
				virtual void OnResponse() = 0;

			private:
				friend class Lv2Plugin;
				void Work(LV2_Worker_Respond_Function respond, LV2_Worker_Respond_Handle handle);
				virtual void Response();
			};

			/// @brief A worker action with provisions for deleting discarded objects from the audo thread on the background thread.
			///
			/// Similar to @ref WorkerAction, but with two additional callbacks, @ref OnCleanup (which
			/// gets executed on the background thread, and @ref OnCleanupComplete (which gets executed
			/// on the Audio thread).
			///
			/// Often background actions will deposit objects for use on the audio thread and must
			/// then de-allocated the previous object. WorkerActionWithCleanup is designed with this
			/// scenario in mind.
			///
			/// The intended use is something like this:
			///
			/// OnWork: - Build complex objects on the background thread.
			///
			/// OnComplete: - On the audio thread, install those objects for use on the audio thread, capturing old objects for subsequent deletion.
			///
			/// OnCleanup: On the background thread, delete the old objects.
			///
			/// OnCleanupComplete: On the audio thread, see if there are new pending requests, and start again if neccesary.

			class WorkerActionWithCleanup : public WorkerAction
			{
			public:
				WorkerActionWithCleanup(Lv2Plugin *plugin)
					: WorkerAction(plugin),
					  cleanupWorker(plugin, this)

				{
				}

			protected:
				virtual void OnCleanup() = 0;
				virtual void OnCleanupComplete() = 0;

			private:
				class CleanupWorker : public WorkerAction
				{
				public:
					CleanupWorker(Lv2Plugin *plugin, WorkerActionWithCleanup *pThis);

				private:
					virtual void OnWork();
					virtual void OnResponse();

				private:
					WorkerActionWithCleanup *pThis;
				};
				CleanupWorker cleanupWorker;

				void Response()
				{
					WorkerAction::Response();
					cleanupWorker.Request();
				}
			};

		protected:
			virtual LV2_Worker_Status OnWork(
				LV2_Worker_Respond_Function respond,
				LV2_Worker_Respond_Handle handle,
				uint32_t size,
				const void *data)
			{
				assert(size == sizeof(WorkerAction *));
				WorkerAction *pWorker = *(WorkerAction **)data;
				pWorker->Work(respond, handle);
				return LV2_Worker_Status::LV2_WORKER_SUCCESS;
			}

			virtual LV2_Worker_Status OnWorkResponse(uint32_t size, const void *data)
			{
				assert(size == sizeof(WorkerAction *));
				WorkerAction *worker = *(WorkerAction **)data;
				worker->Response();
				return LV2_Worker_Status::LV2_WORKER_SUCCESS;
			}

		protected:
			const LV2_Worker_Schedule *GetLv2WorkerSchedule() const
			{
				return schedule;
			}

		private:

			friend LV2_Descriptor **GetLv2GetDescriptors();
			double rate;
			LV2_Log_Logger logger;
			LV2_Worker_Schedule *schedule = nullptr;
			LV2_Options_Option *options = nullptr;
			LV2_Atom_Forge_Frame outputFrame;
			LV2_Atom_Forge inputForge;

			BufSizeOptions bufSizeOptions;
			static Lv2LogLevel logLevel;
			std::string bundle_path;
			bool hasState = false;

		public:
			static LV2_Descriptor **GetLv2GetDescriptors();

		private:
			static LV2_Handle
			instantiate(const LV2_Descriptor *descriptor,
						double rate,
						const char *bundle_path,
						const LV2_Feature *const *features);
			static void connect_port(LV2_Handle instance, uint32_t port, void *data);

			static void activate(LV2_Handle instance);
			static void run(LV2_Handle instance, uint32_t n_samples);
			static void deactivate(LV2_Handle instance);
			static void cleanup(LV2_Handle instance);

			static LV2_Worker_Status work_response(LV2_Handle instance, uint32_t size, const void *data);

			static LV2_Worker_Status work(
				LV2_Handle instance,
				LV2_Worker_Respond_Function respond,
				LV2_Worker_Respond_Handle handle,
				uint32_t size,
				const void *data);

			static LV2_State_Status save(
				LV2_Handle instance,
				LV2_State_Store_Function store,
				LV2_State_Handle handle,
				uint32_t flags,
				const LV2_Feature *const *features);

			static LV2_State_Status restore(
				LV2_Handle instance,
				LV2_State_Retrieve_Function retrieve,
				LV2_State_Handle handle,
				uint32_t flags,
				const LV2_Feature *const *features);

			static const void *extension_data(const char *uri);
			static const void *extension_data_with_state(const char *uri);

			int32_t GetIntOption(const LV2_Options_Option *option);
			void InitBufSizeOptions();

			class PluginUrids
			{

			public:
				LV2_URID patch;
				LV2_URID patch__Get;
				LV2_URID patch__Set;
				LV2_URID patch__property;
				LV2_URID patch__accept;
				LV2_URID patch__value;
				LV2_URID atom__URID;
				LV2_URID atom__Float;
				LV2_URID atom__Int;
				LV2_URID atom__String;
				LV2_URID atom__Path;
				LV2_URID units__frame;
				LV2_URID state__StateChanged;
				LV2_URID buf_size__maxBlockLength;
				LV2_URID buf_size__minBlockLength;
				LV2_URID buf_size__nominalBlockLength;
				LV2_URID buf_size__sequenceSize;

				void Init(LV2_URID_Map *map)
				{
					patch = map->map(map->handle, LV2_PATCH_URI);
					patch__Get = map->map(map->handle, LV2_PATCH__Get);
					patch__Set = map->map(map->handle, LV2_PATCH__Set);
					patch__property = map->map(map->handle, LV2_PATCH__property);
					patch__accept = map->map(map->handle, LV2_PATCH__accept);
					patch__value = map->map(map->handle, LV2_PATCH__value);
					atom__URID = map->map(map->handle, LV2_ATOM__URID);
					atom__Float = map->map(map->handle, LV2_ATOM__Float);
					atom__Int = map->map(map->handle, LV2_ATOM__Int);
					atom__String = map->map(map->handle, LV2_ATOM__String);
					atom__Path = map->map(map->handle, LV2_ATOM__Path);
					units__frame = map->map(map->handle, LV2_UNITS__frame);
					state__StateChanged = map->map(map->handle, LV2_STATE__StateChanged);
					buf_size__minBlockLength = map->map(map->handle, LV2_BUF_SIZE__minBlockLength);
					buf_size__maxBlockLength = map->map(map->handle, LV2_BUF_SIZE__maxBlockLength);
					buf_size__nominalBlockLength = map->map(map->handle, LV2_BUF_SIZE__nominalBlockLength);
					buf_size__sequenceSize = map->map(map->handle, LV2_BUF_SIZE__sequenceSize);
				}
			};

			PluginUrids urids;

			std::vector<LV2_URID> patchPropertyUrids;

			LV2_Atom_Sequence *controlInput = nullptr; 
			LV2_Atom_Sequence *controlOutput = nullptr;
			void RunOuter(uint32_t n_samples) 
			{
				if (controlOutput)
				{
					BeginAtomOutput(controlOutput);
				}
				if (controlInput) {
					HandleEvents(controlInput);
				}
				Run(n_samples);
			}

		};
		////////////////////////////////////////////////////////
		class Lv2PluginWithState : public Lv2Plugin
		{
		public:
			static constexpr bool HAS_STATE = true;

			Lv2PluginWithState(double rate, const char *bundlePath, const LV2_Feature *const *features) : Lv2Plugin(rate, bundlePath, features, true) {}
			static bool HasState() { return true; }
		};
		////////////////////////////////////////////////////////////////////

		template <typename T>
		inline const T *Lv2Plugin::GetFeature(const LV2_Feature *const *features, const char *featureUri)
		{
			while (*features != nullptr)
			{
				if (strcmp((*features)->URI, featureUri) == 0)
				{
					return (const T *)((*features)->data);
				}
				++features;
			}
			return nullptr;
		}

		class PluginRegistrationBase
		{
		protected:
			PluginRegistrationBase(const std::string &pluginUri);

		public:
			~PluginRegistrationBase();
			const std::string &getPluginuri() const;

			virtual Lv2Plugin *Create(double rate, const char *bundlePath, const LV2_Feature *const *features) = 0;
			virtual bool hasState() const = 0;

		private:
			std::string pluginUri;
		};

		template <typename T>
			requires std::derived_from<T, Lv2Plugin>
		class PluginRegistration : public PluginRegistrationBase
		{
		public:
			using super = PluginRegistrationBase;
			PluginRegistration(const std::string &pluginUri) : super(pluginUri) {}
			virtual Lv2Plugin *Create(double rate, const char *bundlePath, const LV2_Feature *const *features) override
			{
				return new T(rate, bundlePath, features);
			}
			virtual bool hasState() const override
			{
				return T::HAS_STATE;
			}
		};

	} // namesapce
} // namespace
