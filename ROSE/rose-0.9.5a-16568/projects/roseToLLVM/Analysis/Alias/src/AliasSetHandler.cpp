#include <AliasSetHandler.h>

AliasSetHandler* AliasSetHandler::_handler = NULL;

void AliasSetHandler::addNewModule(std::string modulename)
{
    AliasSetContainerList *list = new AliasSetContainerList(modulename);
    _mlist[modulename] = list;
}

AliasSetHandler::~AliasSetHandler()
{
    std::map<std::string, AliasSetContainerList*>::iterator I;
    for(I = _mlist.begin(); I != _mlist.end(); ++I) {
        delete I->second;
    }

    delete _handler;
}

void AliasSetHandler::print()
{
    std::map<std::string, AliasSetContainerList*>::iterator I;
    for(I = _mlist.begin(); I != _mlist.end(); ++I) {
        I->second->print();
    }
}

void AliasSetHandler::processAliasInformation(std::string _modulename)
{
    AliasSetContainerList *list = _mlist.find(_modulename)->second;
    list->parseAliasSet();
}

AliasSetContainerList* AliasSetHandler::getAliasSetContainerList(std::string modulename)
{
    return _mlist.find(modulename)->second;
}

std::string AliasSetHandler::getModuleName(std::string _functionname)
{
    std::string _modulename;
    AliasSetContainerList *list;
    std::map<std::string, AliasSetContainerList*>::iterator I;
    for(I = _mlist.begin(); I != _mlist.end(); ++I) {
        _modulename = I->first;
        list = I->second;
        if(list->isFunctionPresent(_functionname))
            break;
    }

    return _modulename;
}
