#ifndef CREATE_H
#define CREATE_H

#include <string>

#include "../bessport/module.h"

using std::string;

template<class TMod, class... TModArg>
inline Module* create_module(const string& module_type_name, const string& module_name, TModArg&&... args){
  const ModuleBuilder& builder =
        ModuleBuilder::all_module_builders().find(module_type_name)->second;

  string final_name = module_name;
  if (ModuleBuilder::all_modules().count(module_name)) {
    final_name = ModuleBuilder::GenerateDefaultName(builder.class_name(),
                                                    builder.name_template());
  }

  Module* m = builder.CreateModule(final_name, &bess::metadata::default_pipeline);

  static_cast<TMod*>(m)->customized_init(std::forward<TModArg>(args)...);

  return m;
}

#endif
