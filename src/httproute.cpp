#include "httproute.h"

using namespace qttp;

Input::Input() :
  name(),
  description(),
  isRequired(false),
  paramType("query"),
  dataType("string"),
  values(),
  visibility(Visibility::Show)
{
}

Input::Input(Input&& from) :
  name(std::move(from.name)),
  description(std::move(from.description)),
  isRequired(from.isRequired),
  paramType(std::move(from.paramType)),
  dataType(std::move(from.dataType)),
  values(std::move(from.values)),
  visibility(Visibility::Show)
{
}

Input::Input(const Input& from)
{
  name = from.name;
  description = from.description;
  isRequired = from.isRequired;
  paramType = from.paramType;
  dataType = from.dataType;
  values = from.values;
  visibility = from.visibility;
}

Input::Input(const QString& inputname) :
  name(inputname),
  description(),
  isRequired(false),
  paramType("query"),
  dataType("string"),
  values(),
  visibility(Visibility::Show)
{
}

Input::Input(const QString& inputname, const QString& desc, const QStringList &vals) :
  name(inputname),
  description(desc),
  isRequired(false),
  paramType("query"),
  dataType("string"),
  values(vals),
  visibility(Visibility::Show)
{
}

Input& Input::operator=(const Input& from)
{
  name = from.name;
  description = from.description;
  isRequired = from.isRequired;
  paramType = from.paramType;
  dataType = from.dataType;
  values = from.values;
  visibility = from.visibility;
  return *this;
}

RequiredInput::RequiredInput() :
  Input()
{
}

RequiredInput::RequiredInput(const QString& inputname) :
  Input(inputname)
{
  isRequired = true;
}

RequiredInput::RequiredInput(const QString& inputname, const QString& desc, const QStringList& vals) :
  Input(inputname, desc, vals)
{
  isRequired = true;
}

HeaderInput::HeaderInput() :
  Input()
{
  paramType = "header";
}

HeaderInput::HeaderInput(const QString& inputname) :
  Input(inputname)
{
  paramType = "header";
}

HeaderInput::HeaderInput(const QString& inputname, const QString& desc, const QStringList& vals) :
  Input(inputname, desc, vals)
{
  paramType = "header";
}
