
#include "stdafx.h"
#include <unordered_map>
#include "ParticleType.h"

ParticleType::ParticleType(const std::string& name, int id, bool overrideLimiter, int paramCount)
{
    this->name            = name;           
    this->id              = id;             
    this->overrideLimiter = overrideLimiter; 
    this->paramCount      = paramCount;    
}

const ParticleType* ParticleType::blockdust  = new ParticleType("blockdust_", 0x29, false, 1);
const ParticleType* ParticleType::blockcrack = new ParticleType("blockcrack_", 0x28, false, 2); 

static std::unordered_map<int, const ParticleType*>& particleRegistry()
{
    static std::unordered_map<int, const ParticleType*> registry = {
        { ParticleType::blockdust->getId(),  ParticleType::blockdust },
        { ParticleType::blockcrack->getId(), ParticleType::blockcrack },
    };
    return registry;
}


int ParticleType::getId() const
{
    return id; 
}

bool ParticleType::getOverrideLimiter() const
{
    return overrideLimiter; 
}

int ParticleType::getParamCount() const
{
    return paramCount;
}


const ParticleType* ParticleType::getDefault()
{
   
    return nullptr;
}

const ParticleType* ParticleType::byId(int searchId)
{
    auto& reg = particleRegistry();
    auto it = reg.find(searchId);
    return (it != reg.end()) ? it->second : nullptr;
}