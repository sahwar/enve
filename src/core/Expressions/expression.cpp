#include "expression.h"

#include "exceptions.h"

Expression::ResultTester Expression::sQrealAnimatorTester =
        [](const QJSValue& val) {
            if(!val.isNumber()) PrettyRuntimeThrow("Invalid return type");
        };

Expression::Expression(const QString& definitionsStr,
                       const QString& scriptStr,
                       PropertyBindingMap&& bindings,
                       std::unique_ptr<QJSEngine>&& engine,
                       QJSValue&& eEvaluate) :
    mDefinitionsStr(definitionsStr),
    mScriptStr(scriptStr),
    mEEvaluate(std::move(eEvaluate)),
    mBindings(std::move(bindings)),
    mEngine(std::move(engine)) {}


void throwIfError(const QJSValue& value, const QString& name) {
    if(value.isError()) {
        PrettyRuntimeThrow("Uncaught exception in " + name + " at line "
                           + value.property("lineNumber").toString() +
                           ":\n" + value.toString());
    }
}

void Expression::sAddDefinitionsTo(const QString& definitionsStr,
                                   QJSEngine& e) {
    const auto defRet = e.evaluate(definitionsStr);
    throwIfError(defRet, "Definitions");
}

void Expression::sAddScriptTo(const QString& scriptStr,
                              const PropertyBindingMap& bindings,
                              QJSEngine& e, QJSValue& eEvaluate,
                              const ResultTester& resultTester) {
    QStringList bindingVars;
    QJSValueList testArgs;
    for(const auto& binding : bindings) {
        bindingVars << binding.first;
        testArgs << binding.second->getJSValue(e);
    }
    const QString evalVars = bindingVars.join(", ");
    const auto scrRet = e.evaluate(
            "var eEvaluate = function(" + evalVars + ") {" +
                scriptStr +
            "}");
    throwIfError(scrRet, "Script");
    eEvaluate = e.evaluate("eEvaluate");
    throwIfError(eEvaluate, "Script");
    if(!eEvaluate.isCallable())
        PrettyRuntimeThrow("Uncallable script.");
    const auto testResult = eEvaluate.call(testArgs);
    if(testResult.isError()) {
        PrettyRuntimeThrow("Script test error:\n" +
                           testResult.toString());
    } else if(resultTester) resultTester(testResult);
}

qsptr<Expression> Expression::sCreate(const QString& bindingsStr,
                                      const QString& definitionsStr,
                                      const QString& scriptStr,
                                      const Property* const context,
                                      const ResultTester& resultTester) {
    auto bindings = PropertyBindingParser::parseBindings(
                              bindingsStr, nullptr, context);
    auto engine = std::make_unique<QJSEngine>();
    sAddDefinitionsTo(definitionsStr, *engine);
    QJSValue eEvaluate;
    sAddScriptTo(scriptStr, bindings, *engine, eEvaluate, resultTester);
    return sCreate(definitionsStr, scriptStr,
                   std::move(bindings),
                   std::move(engine),
                   std::move(eEvaluate));
}

qsptr<Expression> Expression::sCreate(const QString& definitionsStr,
                                      const QString& scriptStr,
                                      PropertyBindingMap&& bindings,
                                      std::unique_ptr<QJSEngine>&& engine,
                                      QJSValue&& eEvaluate) {
    if(!eEvaluate.isCallable())
        RuntimeThrow("Uncallable script:\n" + scriptStr);
    return qsptr<Expression>(new Expression(definitionsStr, scriptStr,
                                            std::move(bindings),
                                            std::move(engine),
                                            std::move(eEvaluate)));
}

bool Expression::setAbsFrame(const qreal absFrame) {
    bool changed = false;
    for(const auto& binding : mBindings) {
        changed = changed || binding.second->setAbsFrame(absFrame);
    }
    return changed;
}

bool Expression::isValid() {
    for(const auto& binding : mBindings) {
        const bool valid = binding.second->isBindPathValid();
        if(!valid) return false;
    }
    return true;
}

bool Expression::dependsOn(const Property* const prop) {
    for(const auto& binding : mBindings) {
        const bool depends = binding.second->dependsOn(prop);
        if(depends) return true;
    }
    return false;
}

QJSValue Expression::evaluate() {
    QJSValueList values;
    for(const auto& binding : mBindings) {
        values << binding.second->getJSValue(*mEngine);
    }
    return mEEvaluate.call(values);
}

QJSValue Expression::evaluate(const qreal relFrame) {
    QJSValueList values;
    for(const auto& binding : mBindings) {
        values << binding.second->getJSValue(*mEngine, relFrame);
    }
    return mEEvaluate.call(values);
}

FrameRange Expression::identicalRange(const qreal absFrame) {
    FrameRange result = FrameRange::EMINMAX;
    for(const auto& binding : mBindings) {
        result *= binding.second->identicalRange(absFrame);
    }
    return result;
}

QString Expression::bindingsString() const {
    QString result;
    for(const auto& binding : mBindings) {
        result += binding.first + " = " + binding.second->path() + ";\n";
    }
    return result;
}
