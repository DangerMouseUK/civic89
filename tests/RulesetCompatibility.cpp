// Civic 89 M7 mode and city compatibility acceptance. SPDX-License-Identifier: GPL-3.0-or-later
#include "Budget.h"
#include "CityDocument.h"
#include "CityProperties.h"
#include "EngineDigest.h"
#include "EngineState.h"
#include "FileIo.h"
#include "RecoveryStore.h"
#include "ScenarioData.h"
#include "WindowsFileStorage.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace
{
    void require(bool condition,const char* message) { if (!condition) { throw std::runtime_error(message); } }
    std::vector<char> read(const std::filesystem::path& path)
    {
        std::ifstream input(path,std::ios::binary);
        return {std::istreambuf_iterator<char>(input),{}};
    }
    void write(const std::filesystem::path& path,std::span<const char> bytes)
    {
        std::ofstream output(path,std::ios::binary | std::ios::trunc);
        output.write(bytes.data(),static_cast<std::streamsize>(bytes.size()));
        require(static_cast<bool>(output),"Cannot write test fixture");
    }
    class FailedWriter : public AtomicFileWriter
    {
    public:
        int calls{};
        CityIoResult write(const std::filesystem::path& path,std::span<const char>) override
            { ++calls; return {CityIoCode::WriteFailed,path,"Injected short write"}; }
    };
}

int main(int argc,char** argv)
{
    const auto directory=std::filesystem::temp_directory_path() / ("civic89-m7-" +
        std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    struct Cleanup { std::filesystem::path path; ~Cleanup() { std::error_code error; std::filesystem::remove_all(path,error); } } cleanup{directory};
    try
    {
        require(argc==2,"Expected fixture root");
        require(std::filesystem::create_directory(directory),"Cannot create isolated compatibility directory");
        const std::filesystem::path root=argv[1];
        Budget budget; CityProperties city; WindowsFileStorage storage;
        initializeEngine(city,budget);
        require(city.rulesetId()==RulesetId::ClassicV1,"Default session is not Classic v1");
        require(findRuleset(1,1)==findRuleset(RulesetId::ClassicV1) &&
            findRuleset(2,1)==findRuleset(RulesetId::EnhancedV1),"Versioned registry identity differs");
        require(!findRuleset(2,2) && !findRuleset(99,1),"Unknown rule versions accepted");
        const auto before=engineStateDigest(city,budget);
        bool rejected=false;
        try { initializeEngine(city,budget,static_cast<RulesetId>(99)); } catch (const std::invalid_argument&) { rejected=true; }
        require(rejected && city.rulesetId()==RulesetId::ClassicV1 && engineStateDigest(city,budget)==before,
            "Unknown session definition mutated the active city");
        require(!city.rulesetId(static_cast<RulesetId>(99)),"City identity accepted an unknown definition");

        // Independently specified schema/IEEE CRC fixture (Python struct/zlib),
        // so matching encoder/reader mistakes cannot redefine the wire format.
        const CityDocument golden{RulesetId::EnhancedV1,"Golden city",std::vector<char>(51360)};
        std::vector<char> goldenBytes;
        require(static_cast<bool>(encodeCityDocument(golden,goldenBytes)),"Cannot encode wire-format reference");
        const std::array<unsigned char,44> header{
            'C','I','V','I','C','8','9',0, 1,0,0,0, 2,0,0,0, 1,0,0,0,
            120,0,0,0, 100,0,0,0, 11,0,0,0, 0xa0,0xc8,0,0, 0,0,0,0, 0xe4,0x66,0xac,0x8d};
        require(goldenBytes.size()==51415 && std::equal(header.begin(),header.end(),goldenBytes.begin(),
            [](unsigned char expected,char actual) { return expected==static_cast<unsigned char>(actual); }),
            "Enhanced header/checksum differs from schema-1 reference");

        city.GameLevel(2); city.CityName("Oldtown"); budget.CurrentFunds(765432); budget.TaxRate(13); CityTime=321;
        ResidentialPopulationHistory.fill(42); tileValue(60,50)=RoadHorizontal;
        const auto classic=directory/L"original-\u57ce.cty";
        require(static_cast<bool>(SaveCity(classic,city,budget,storage)),"Classic publication failed");
        const auto original=read(classic);
        require(original.size()==ScenarioFileSize,"Classic byte size changed");
        require(static_cast<bool>(ImportClassicCity(classic,city,budget)) && city.rulesetId()==RulesetId::EnhancedV1,
            "Explicit Classic import failed");
        require(read(classic)==original,"Import modified the source city");
        const auto importedName=city.CityName();
        const auto enhanced=directory/L"wrapped-\u57ce.C89";
        require(static_cast<bool>(SaveCity(enhanced,city,budget,storage)),"Enhanced Unicode save failed");
        CityDocument document;
        require(static_cast<bool>(readCityDocument(enhanced,document)) && document.ruleset==RulesetId::EnhancedV1 &&
            document.name==importedName,"Enhanced identity/name was not recorded");
        const auto exported=directory/"explicit-copy.cty";
        const auto snapshot=engineStateDigest(city,budget);
        require(static_cast<bool>(ExportClassicCity(exported,city,budget,storage)) && read(exported)==document.classicPayload,
            "Classic export changed the embedded payload/layout");
        require(city.rulesetId()==RulesetId::EnhancedV1 && engineStateDigest(city,budget)==snapshot,
            "Export mutated the active mode/simulation");
        FailedWriter failed;
        require(SaveCity(classic,city,budget,failed).code==CityIoCode::IncompatibleMode && failed.calls==0 && read(classic)==original,
            "Ordinary Enhanced save attempted to overwrite Classic");
        const auto goodBytes=read(enhanced);
        require(SaveCity(enhanced,city,budget,failed).code==CityIoCode::WriteFailed && read(enhanced)==goodBytes &&
            engineStateDigest(city,budget)==snapshot && city.rulesetId()==RulesetId::EnhancedV1,"Failed Enhanced write changed data/state");
        const HANDLE lock=CreateFileW(enhanced.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        require(lock!=INVALID_HANDLE_VALUE,"Cannot lock Enhanced destination");
        const auto publication=SaveCity(enhanced,city,budget,storage);
        CloseHandle(lock);
        require(publication.code==CityIoCode::ReplaceFailed && read(enhanced)==goodBytes,
            "Failed Enhanced replacement damaged the earlier document");
        for (const auto& entry : std::filesystem::directory_iterator(directory))
            { require(pathUtf8(entry.path()).find(".tmp-")==std::string::npos,"Failed publication leaked a temporary file"); }

        const auto renamed=directory/"renamed.c89";
        write(renamed,goodBytes);
        initializeEngine(city,budget);
        require(static_cast<bool>(LoadCityDetailed(renamed,city,budget)) && city.rulesetId()==RulesetId::EnhancedV1 &&
            city.CityName()==importedName && city.GameLevel()==2 && budget.CurrentFunds()==765432 && CityTime==321 &&
            ResidentialPopulationHistory[0]==42,"Enhanced round trip lost identity/name/history/difficulty/funds");
        const auto intact=engineStateDigest(city,budget);
        const auto bad=directory/"rejected.c89";
        const auto reject=[&](const std::vector<char>& bytes,CityIoCode code)
        {
            write(bad,bytes);
            require(LoadCityDetailed(bad,city,budget).code==code,"Malformed Enhanced input reported wrong result");
            require(city.rulesetId()==RulesetId::EnhancedV1 && city.CityName()==importedName && engineStateDigest(city,budget)==intact,
                "Rejected Enhanced document changed live state/mode/name");
        };
        for (size_t size : {size_t(0),size_t(43),goodBytes.size()-1,goodBytes.size()+1})
            { auto bytes=goodBytes; bytes.resize(size); reject(bytes,CityIoCode::InvalidFormat); }
        reject(std::vector<char>(1000000),CityIoCode::InvalidFormat);
        for (size_t offset : {size_t(0),size_t(8),size_t(20),size_t(28),size_t(32),size_t(36),size_t(40),goodBytes.size()-1})
            { auto bytes=goodBytes; bytes[offset]^=1; reject(bytes,CityIoCode::InvalidFormat); }
        for (size_t offset : {size_t(12),size_t(16)})
            { auto bytes=goodBytes; bytes[offset]=99; reject(bytes,CityIoCode::UnsupportedRuleset); }
        auto invalid=document;
        const int negativeTime=-1;
        std::memcpy(invalid.classicPayload.data()+6*sizeof(GraphHistory)+8*sizeof(int),&negativeTime,sizeof(int));
        std::vector<char> encoded;
        require(static_cast<bool>(encodeCityDocument(invalid,encoded)),"Cannot encode semantic rejection fixture");
        reject(encoded,CityIoCode::InvalidData); // Valid container checksum, invalid engine fields.
        invalid=document;
        const int tile=TILE_COUNT;
        std::memcpy(invalid.classicPayload.data()+7*sizeof(GraphHistory),&tile,sizeof(int));
        require(static_cast<bool>(encodeCityDocument(invalid,encoded)),"Cannot encode tile rejection fixture");
        reject(encoded,CityIoCode::InvalidFormat);
        for (const std::string& name : {std::string{},std::string(256,'x'),std::string("a\0b",3),std::string("\xc0\x80"),std::string("\xed\xa0\x80")})
        {
            invalid=document; invalid.name=name;
            require(encodeCityDocument(invalid,encoded).code==CityIoCode::InvalidData,"Invalid UTF-8/name accepted");
        }
        RulesetId inspected=RulesetId::ClassicV1;
        require(InspectCity(bad,inspected).code==CityIoCode::InvalidFormat && inspected==RulesetId::ClassicV1 &&
            engineStateDigest(city,budget)==intact,"Inspection mutated output or engine on failure");
        require(!ImportClassicCity(enhanced,city,budget) && city.rulesetId()==RulesetId::EnhancedV1 &&
            engineStateDigest(city,budget)==intact,"Enhanced document accepted as Classic import");
        std::string longName;
        for (int i=0;i<86;++i) { longName+="\xe5\x9f\x8e"; }
        const auto unrepresentable=directory/pathFromUtf8(longName+".cty");
        write(unrepresentable,original);
        require(ImportClassicCity(unrepresentable,city,budget).code==CityIoCode::InvalidData &&
            city.rulesetId()==RulesetId::EnhancedV1 && engineStateDigest(city,budget)==intact,
            "Unrepresentable import name changed the city before rejection");
        require(static_cast<bool>(LoadCityDetailed(classic,city,budget)) && city.rulesetId()==RulesetId::ClassicV1,
            "Opening a Classic file did not select Classic");
        require(SaveCity(enhanced,city,budget,failed).code==CityIoCode::IncompatibleMode,"Classic silently saved into Enhanced format");
        initializeEngine(city,budget,RulesetId::EnhancedV1);
        require(LoadScenario(Scenario::Detroit,city,budget,root/"scenarios")==ScenarioResult::Success &&
            city.rulesetId()==RulesetId::ClassicV1,"Inherited scenario did not select Classic v1");

        RecoveryStore recovery(directory/"recovery");
        require(!recovery.latest(),"Unexpected recovery slot");
        require(static_cast<bool>(recovery.save(city,budget,storage)),"Classic recovery write failed");
        const auto classicRecovery=read(recovery.path());
        initializeEngine(city,budget,RulesetId::EnhancedV1); city.CityName("Recovered Enhanced"); budget.CurrentFunds(54321);
        require(static_cast<bool>(recovery.save(city,budget,storage)),"Enhanced recovery write failed");
        const auto now=std::filesystem::file_time_type::clock::now();
        std::filesystem::last_write_time(recovery.path(),now-std::chrono::seconds(2));
        std::filesystem::last_write_time(recovery.path(RulesetId::EnhancedV1),now);
        require(recovery.latest()==recovery.path(RulesetId::EnhancedV1) && read(recovery.path())==classicRecovery,
            "Recovery mixed modes or lost the Classic slot");
        require(static_cast<bool>(LoadCityDetailed(*recovery.latest(),city,budget)) && city.rulesetId()==RulesetId::EnhancedV1 &&
            budget.CurrentFunds()==54321,"Enhanced recovery lost its ruleset/funds");
        const auto enhancedRecovery=read(recovery.path(RulesetId::EnhancedV1));
        require(!recovery.save(city,budget,failed) && read(recovery.path(RulesetId::EnhancedV1))==enhancedRecovery,
            "Failed recovery damaged the selected slot");
        write(recovery.path(RulesetId::EnhancedV1),std::span<const char>("corrupt",7));
        require(recovery.latest()==recovery.path(),"Invalid newest Enhanced recovery hid valid Classic recovery");
        std::cout << "M7 versioned modes, Classic bytes, Enhanced container, conversions, atomic failures, rejection and separate recovery passed\n";
        return 0;
    }
    catch (const std::exception& error) { std::cerr << error.what() << '\n'; return 1; }
}
