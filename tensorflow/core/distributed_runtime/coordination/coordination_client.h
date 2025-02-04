/* Copyright 2021 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#ifndef TENSORFLOW_CORE_DISTRIBUTED_RUNTIME_COORDINATION_COORDINATION_CLIENT_H_
#define TENSORFLOW_CORE_DISTRIBUTED_RUNTIME_COORDINATION_COORDINATION_CLIENT_H_

#include <memory>
#include <string>

namespace tensorflow {

// Client interface for communicating with coordination service.
// Experimental feature, not yet implemented in open source.
class CoordinationClient;

// Simple wrapper class that can be used to retrieve CoordinationClients.
class CoordinationClientCache {
 public:
  virtual ~CoordinationClientCache() {}

  // If the `target` names a remote task, returns a pointer of the
  // CoordinationClient object wrapping that channel to the remote task.
  virtual CoordinationClient* GetClient(const std::string& target) = 0;

  // If the `target` names a remote task, returns an owned pointer of the
  // CoordinationClient object wrapping that channel to the remote task.
  virtual std::unique_ptr<CoordinationClient> GetOwnedClient(
      const std::string& target) = 0;
};

}  // namespace tensorflow

#endif  // TENSORFLOW_CORE_DISTRIBUTED_RUNTIME_COORDINATION_COORDINATION_CLIENT_H_
