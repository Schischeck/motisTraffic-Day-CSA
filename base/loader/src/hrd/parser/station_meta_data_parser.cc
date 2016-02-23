#include "motis/loader/hrd/parser/station_meta_data_parser.h"

#include <cinttypes>
#include <string>
#include <vector>
#include <array>
#include <functional>

#include "parser/csv.h"
#include "parser/cstr.h"
#include "parser/util.h"

namespace motis {
namespace loader {
namespace hrd {

using namespace parser;

constexpr int DEFAULT_CHANGE_TIME_LONG_DISTANCE = 5;
constexpr int DEFAULT_CHANGE_TIME_LOCAL_TRANSPORT = 2;

//   0: <Gültig-Ab-Datum>
//   1: <Gelöscht-Flag>
//   2: <Name>
// * 3: <EVA-Nummer>
// * 4: <[RL100-Code 1, ...,RL100-Code N]>
//   5: <IBNR>
//   6: <UIC-Nummer>
//   7: <Staat>
//   8: <Amtlicher-Gemeinde-schlüssel>
//   9: <Langname>
//   10: <KFZ-Kennzeichen>
//   11: <Kurzname_1>
//   12: ...
//   13: <Kurzname_n>
void parse_ds100_mappings(loaded_file const& infotext_file,
                          std::map<cstr, int>& ds100_to_eva_number) {
  enum { eva_number, ds100_code };
  typedef std::tuple<int, cstr> entry;

  std::array<uint8_t, detail::MAX_COLUMNS> column_map;
  std::fill(begin(column_map), end(column_map), detail::NO_COLUMN_IDX);
  column_map[3] = 0;
  column_map[4] = 1;

  std::vector<entry> entries;
  auto next = infotext_file.content();
  while (next) {
    next = skip_lines(next, [](cstr const& line) { return line.len < 38; });
    if (!next) {
      break;
    }
    auto row = detail::read_row<entry, ':'>(next, column_map, 5);
    entry e;
    read(e, row);
    auto const eva_num = std::get<eva_number>(e);
    for_each_token(std::get<ds100_code>(e), ',',
                   [&ds100_to_eva_number, &eva_num](cstr token) {
                     ds100_to_eva_number[token] = eva_num;
                   });
  }
}

enum { from_ds100_key, to_ds100_key, duration_key, platform_change_time_key };
typedef std::tuple<cstr, cstr, int, int> minct;
void load_minct(std::vector<minct>& records) {
  loaded_file minct_file{"minct.csv", station_meta_data::MINCT};
  std::array<detail::column_idx_t, detail::MAX_COLUMNS> column_map;
  std::fill(begin(column_map), end(column_map), detail::NO_COLUMN_IDX);
  column_map[0] = 0;
  column_map[1] = 1;
  column_map[2] = 2;
  column_map[3] = 3;
  cstr minct_content(station_meta_data::MINCT);
  auto rows = detail::read_rows<minct, ';'>(minct_content, column_map);
  read(records, rows);
}

int station_meta_data::get_station_change_time(int eva_num) const {
  auto it = station_change_times_.find(eva_num);
  if (it == std::end(station_change_times_)) {
    if (eva_num < 1000000) {
      return DEFAULT_CHANGE_TIME_LOCAL_TRANSPORT;
    } else {
      return DEFAULT_CHANGE_TIME_LONG_DISTANCE;
    }
  } else {
    return it->second;
  }
}

void parse_and_add(loaded_file const& metabhf_file,
                   std::set<station_meta_data::footpath>& footpaths,
                   std::set<station_meta_data::meta_station>& meta_stations) {
  for_each_line(metabhf_file.content(), [&](cstr line) {
    if (line.length() < 19 || line[0] == '%' || line[0] == '*') {
      return;
    }

    if (line[7] == ':') {  // equivalent stations
      int eva = parse<int>(line.substr(0, size(7)));
      std::vector<int> equivalent;
      for_each_token(line.substr(8), ' ', [&equivalent](cstr token) {
        int e = parse<int>(token);
        if (e != 0) {
          equivalent.push_back(e);
        }
      });
      if (equivalent.size() > 0) {
        meta_stations.insert({eva, equivalent});
      }
    } else {  // footpaths
      footpaths.insert({parse<int>(line.substr(0, size(7))),
                        parse<int>(line.substr(8, size(7))),
                        parse<int>(line.substr(16, size(3)))});
    }
  });
}

void parse_station_meta_data(loaded_file const& infotext_file,
                             loaded_file const& metabhf_file,
                             loaded_file const& metabhf_zusatz_file,
                             station_meta_data& metas) {
  parse_ds100_mappings(infotext_file, metas.ds100_to_eva_num_);

  std::vector<minct> records;
  load_minct(records);

  for (auto const& record : records) {
    cstr const from_ds100 = std::get<from_ds100_key>(record);
    cstr const to_ds100 = std::get<to_ds100_key>(record);
    int const duration = std::get<duration_key>(record);

    if (to_ds100.len == 0) {
      auto eva_number_it = metas.ds100_to_eva_num_.find(from_ds100);
      if (eva_number_it != end(metas.ds100_to_eva_num_)) {
        metas.station_change_times_[eva_number_it->second] = duration;
      }
    } else {
      auto from_eva_num_it = metas.ds100_to_eva_num_.find(from_ds100);
      auto to_eva_num_it = metas.ds100_to_eva_num_.find(to_ds100);
      if (from_eva_num_it != end(metas.ds100_to_eva_num_) &&
          to_eva_num_it != end(metas.ds100_to_eva_num_)) {
        metas.footpaths_.insert(
            {from_eva_num_it->second, to_eva_num_it->second, duration});
      }
    }
  }
  parse_and_add(metabhf_file, metas.footpaths_, metas.meta_stations_);
  parse_and_add(metabhf_zusatz_file, metas.footpaths_, metas.meta_stations_);
}

const char* station_meta_data::MINCT = R"(AA;;7;4
ABCH;;6;3
ABG;;5;4
ABLZ;;5;3
ABX;;4;3
ACV;;4;3
ADF;;6;3
AE;;2;-
AEL;;5;3
AF;;5;3
AH;;8;4
AHAR;;7;4
AHI;;5;3
AHM;;5;3
AIZ;;5;3
AK;;6;4
AL;;6;3
ALBG;;6;4
AN;;5;3
ANB;;5;3
AO;;5;3
AP;;4;3
APU;;3;-
AR ;;4;3
AROG;;5;3
AST;;4;3
BAHR;;4;3
BALE;BALX;6;-
BALE;;5;-
BALX;BALE;6;-
BALX;;5;-
BBES;;3;3
BBF;;3;3
BBI;;3;3
BBIG;;5;3
BBRI;;3;3
BBRN;;4;3
BC;;3;3
BCS;;5;3
BDKO;;5;3
BEB;;3;3
BERK;;5;3
BEW;;4;3
BFBI;;8;4
BFHS;;6;3
BFIN;;3;3
BFKS;;3;3
BFO;;4;3
BFP;;5;3
BFUW;;4;3
BFW;;3;3
BGS;;6;4
BHBF;BL;8;-
BHBF;BLS;9;-
BHBF;;9;-
BHC;;4;3
BHF;;7;4
BHND;;5;3
BHSH;;4;3
BHW;;4;3
BJUE;;5;3
BJUN;;4;3
BKAR;;3;3
BKH;;4;3
BKI;;3;3
BKW;;5;3
BL;BHBF;8;-
BL;BLS;9;-
BL;;9;4
BLO;;7;4
BLS;BHBF;9;-
BLS;BL;9;-
BLS;;9;4
BMIP;;3;3
BOR;;4;3
BPAF;;7;4
BPD;;6;4
BRU;;4;3
BSN;;4;3
BSPD;;6;4
BST;;5;3
BSWP;;4;3
BUK;;3;3
BVLT;;4;3
BWE;;3;3
BWR;;3;3
BWS;;5;3
BWUS;;3;3
BZOO;;6;4
DAD;;4;3
DAF;;5;3
DAU;;4;3
DBW;;4;3
DC;;5;3
DCW;;4;3
DDE;;4;3
DEG;;4;3
DFA;;3;3
DFL;;5;3
DFR;;5;3
DG;;5;3
DGC;;3;-
DGL;;4;3
DGLS;;5;-
DGZ;;3;3
DH;;8;4
DHD;;5;3
DHN;;4;3
DJ;;3;3
DKT;;4;3
DLH;;3;-
DLI;;3;3
DME;;3;3
DMEH;;4;3
DMHD;;4;-
DMT;;3;3
DN;DN  A;12;-
DN;;5;4
DN  A;DN;12;-
DN  A;;4;-
DNR;;4;3
DP;;4;3
DPF;;3;3
DPI;;5;3
DPR;;5;-
DR;;5;3
DRAG;;4;-
DSA;;5;3
DSEB;;4;3
DTH;;4;3
DWR;;4;3
DWS;;4;3
DZ;;5;3
DZA;;3;-
DZW;;6;3
EBIL;;7;4
EBO;;7;4
EBRW;;5;3
EBTH;;4;3
EBWE;;4;3
EBWG;;4;3
EBZ;;4;3
ECMF;;4;3
EDDP;;4;3
EDG;;7;4
EDMG;;4;3
EDO;;7;4
EDRN;;4;3
EDUL;;6;3
EE;;7;4
EEK;;4;3
EEST;;4;3
EFOE;;5;3
EFP;;6;3
EG;;6;4
EGRN;;4;3
EHER;;4;3
EHFD;;5;3
EHG;;6;4
EHLT;;4;3
EHM;;7;4
EHZW;;4;3
EIL;;3;3
EKT;;4;3
ELAG;;4;3
ELE;;5;3
ELUE;;4;3
EMLR;;5;3
EMRY;;4;3
EMST;;7;4
EOB;;7;4
EPD;;5;3
ERDW;;4;3
ESIE;;5;3
ESOT;;4;3
ESRT;;4;3
ESW;;4;3
EUN;;4;3
EWAN;;6;3
EWES;;6;3
EWIT;;4;3
FALZ;;4;3
FARM;;4;3
FB;;5;3
FBA ;;3;3
FBAH;;3;3
FBEI;;3;3
FBGK;;5;3
FBGN;;5;3
FBH;;5;3
FBHF;;5;3
FBL;;4;3
FBSO;;3;3
FBUE;;6;-
FBUS;;4;3
FD;;7;3
FDI;;4;3
FDIL;;5;3
FDN;;5;3
FEN;;5;3
FF;FFT;10;-
FF;;8;4
FFAW;;2;-
FFG;;5;3
FFLF;FFLU;15;-
FFLF;;7;4
FFLU;FFLF;15;-
FFLU;;6;4
FFRI;;4;3
FFS;;6;4
FFT;FF;10;-
FFT;;4;3
FFU;;7;4
FFW;;5;3
FG;;6;4
FGAL;;4;3
FGEL;;5;3
FGHO;;4;3
FH;;6;4
FHG;;6;3
FHO;;4;3
FHOE;;5;3
FHWD;;4;3
FK;;5;3
FKOB;;4;3
FKW;;7;4
FL;;4;3
FMB;;4;3
FMBG;;5;3
FMS;;4;3
FMSH;;4;3
FMT;;5;3
FMZ;;7;4
FND;;4;3
FNH;;3;3
FNSD;;4;3
FO;;6;3
FOBI;;4;3
FOO;;4;3
FORD;;4;3
FOST;;2;-
FRH;;4;3
FSP;;5;3
FSTM;;4;3
FSUE;;5;3
FTS;;5;3
FVL;;4;3
FW;;5;4
FWAB;;4;3
FWH;;3;3
FWO;;4;3
FWOR;;4;3
FWR;;4;3
HA;;5;3
HB;;8;4
HBB;;4;3
HBDE;;4;3
HBDG;;3;3
HBH;;5;3
HBHA;;3;3
HBHL;;3;3
HBKL;;3;3
HBML;;4;-
HBNT;;4;3
HBOF;;3;3
HBRM;;4;3
HBS;;6;4
HBTH;;4;3
HBV;;3;3
HBWU;;3;3
HC;;6;3
HD;;4;3
HE;;6;3
HEBG;;4;3
HELZ;;5;3
HG;;7;4
HGDN;;3;3
HGI;;4;3
HGS;;4;3
HH;;8;4
HHAS;;5;3
HHB;;3;3
HHBI;;4;3
HHI;;6;4
HHLM;;5;3
HHM;;5;3
HHUD;;4;3
HHZM;;3;3
HK;;5;3
HLEE;;5;3
HLER;;5;3
HLGM;;5;3
HLGW;;4;3
HLON;;5;3
HM;;5;3
HN;;5;3
HNBG;;5;3
HNOS;;4;3
HO;;6;4
HOK;;4;3
HOLD;;6;4
HOTT;;3;3
HR;;6;3
HSAL;;4;3
HSAN;;3;3
HSO;;4;3
HSRI;;4;3
HSSN;;3;3
HU;;6;3
HV;;5;3
HVBG;;4;3
HWAR;;5;3
HWED;;3;3
HWEZ;;5;3
HWOB;;5;4
HWOL;;5;3
HWRN;;3;3
HWUN;;5;3
KA;;6;3
KAK;;3;3
KAND;;6;4
KAU;;4;3
KB;;6;4
KBOP;;5;3
KD;;8;4
KDD;;2;-
KDFF;;8;3
KDN;;5;3
KEU;;4;3
KFKB;;7;4
KGRB;;4;3
KGUI;;3;3
KHEN;;4;3
KHEZ;;4;3
KHR;;4;3
KK;;7;4
KKAS;;6;3
KKDT;KKDZ;7;-
KKDT;KKDZB;7;-
KKDT;;7;-
KKDZ;KKDT;7;-
KKDZ;KKDZB;7;-
KKDZ;;7;4
KKDZB;KKDT;7;-
KKDZB;KKDZ;7;-
KKDZB;;7;-
KKER;;4;3
KKFS;;2;-
KKHR;;2;-
KKM;;4;3
KKO;;6;4
KKR;;4;3
KKRO;;4;3
KKS;;4;3
KKTR;;2;-
KLAW;;4;3
KM;;5;3
KN;;4;3
KNE;;5;3
KNL;;4;3
KPW;;6;3
KRE;;5;3
KRH;;3;3
KRY;;4;3
KSIB;;7;4
KSO;;6;4
KST;;4;3
KT;;5;3
KV;;4;3
KW;;6;4
KWO;;4;3
KWV;;4;3
LA;;4;3
LAL;;4;3
LB;;5;3
LBB;;3;3
LBG;;4;3
LBL;;4;3
LBOR;;4;3
LBT;;5;3
LBZ;;5;3
LCBO;;5;3
LD;;5;3
LDL;LDLO;12;-
LDL;;4;3
LDLO;LDL;12;-
LDLO;;4;-
LEG;;4;3
LF;;5;3
LFLU;;6;-
LGC;;4;3
LGH;;4;3
LGT;;4;3
LGW;;4;3
LH;;7;4
LHB;;4;3
LHT;;3;-
LK;;5;3
LKM;;3;-
LKO;;4;3
LL;;7;4
LL  T;;4;4
LL;LL  T;9;-
LL  T;LL;9;-
LLC;;4;3
LLEL;;4;3
LLG;;6;3
LLN;;3;3
LLP;;4;-
LLPD;;4;3
LLST;;3;3
LLT;;4;-
LM;;6;4
LMB;;4;3
LMDR;;5;-
LMG;;4;3
LMN;;4;3
LNK;;4;3
LNW;;5;3
LOE;;4;3
LR;;4;-
LRW;;5;3
LS;;5;3
LSB;;5;3
LSL;;4;3
LSW;;5;3
LW;;5;3
LWEG;;3;3
LWZ;;4;3
LZ;;4;3
MA;;8;4
MAHZ;;6;4
MAOB;;5;3
MBIH;;4;3
MBU;;5;3
MDA;;5;3
MDS;;5;3
MDT;;5;3
MEB;;5;3
MEG;;3;3
MFL;;6;3
MFR;;5;3
MGA;;3;3
MGB;;5;3
MGE;;5;3
MGP;;5;3
MGZB;;5;3
MH;MH  N;13;-
MH;MH  S;13;-
MH;MHT;10;-
MH;;10;4
MH  N;MH;13;-
MH  N;MH  S;13;-
MH  N;MHT;10;-
MH  N;;6;-
MH  S;MH;13;-
MH  S;MH  N;13;-
MH  S;MHT;10;-
MH  S;;6;-
MHGZ;;4;3
MHO;;5;3
MHT;MH;10;-
MHT;MH  N;10;-
MHT;MH  S;10;-
MIH;;7;4
MIMS;;5;3
MKFB;;4;3
MKFG;;4;3
MKP;;5;3
MKZ;;4;3
MLA;;6;3
MLI;;6;3
MM;;5;3
MMAM;;6;4
MMF;;5;3
MMH;;4;3
MMR;;5;3
MMU;;5;3
MMW;;5;3
MNFR;;5;3
MNL;;4;3
MNM;;4;3
MNR;;4;3
MOP;;7;4
MP;;7;4
MPE;;5;3
MPFS;;3;3
MPR;;5;3
MRO;;6;4
MSB;;5;3
MSBI;;4;3
MTHB;;3;3
MTL;;5;3
MTLG;;3;3
MTS;;5;3
MTZ;;5;3
MWH;;5;3
MWSB;;3;3
NAH;;6;4
NAN;;5;3
NBA;;6;4
NBEI;;4;3
NBG;;3;3
NBKI;;3;3
NBY;;4;3
NC;;4;3
NCH;;5;3
NDHF;;3;3
NEBH;;4;3
NER;;5;3
NF;;6;3
NFO;;4;3
NFT;;4;3
NFW;;4;3
NGM;;6;3
NGUN;;4;3
NHM;;4;3
NHO;;6;3
NHR;;4;3
NHS;;4;3
NK;;4;3
NKL;;4;3
NKZ;;3;-
NLF;;6;3
NLL;;4;3
NM;;3;3
NMBG;;3;3
NMR;;5;3
NN;;8;4
NNA;;5;3
NNE;;5;3
NNKS;;4;3
NNS;;4;3
NNT;;5;3
NOKP;;4;3
NOT;;5;3
NPA;;6;3
NPL;;6;3
NPLF;;5;3
NPZ;;4;3
NRA;;5;3
NRH;;7;4
NRO;;5;3
NRPF;;5;3
NRTD;;5;3
NS;;5;3
NSCH;;5;3
NSDF;;4;3
NST;;5;3
NSTN;;4;3
NWDO;;5;3
NWH;;7;4
NWK;;4;3
NZWL;;4;3
RAH;;5;3
RAP;;6;3
RB;;7;4
RBB;;5;3
RBDH;;3;3
RBI;;4;3
RBKR;;4;3
RBR;;4;3
RBRS;;4;3
RBT;;3;3
RDO;;4;3
RDZ;;4;3
REA;;4;3
RENG;;4;3
REP;;3;3
RF;;7;4
RFHM;;3;3
RFT;;4;3
RGE;;4;3
RGH;;4;3
RGN;;4;3
RGR;;4;3
RH;;6;4
RHA;;4;3
RIM;;5;3
RK;;7;4
RKDU;;5;3
RKO;;5;3
RL;;6;3
RLA;;4;3
RLR;;4;3
RM;;6;4
RMF;;4;3
RMK;;3;3
RN;;5;3
RNSS;;4;3
RNZ;;4;3
RO;;6;4
RRA;;5;3
RRL;;4;3
RRZ;;4;3
RSD;;4;3
RSE;;4;3
RSI;;4;3
RSM;;4;3
RSP;;4;3
RSS;;3;3
RTIT;;5;3
RVL;;4;3
RW;;5;3
RWE;;4;3
RWND;;4;3
RWRT;;4;3
RWU;;4;3
SBKN;;4;3
SBMS;;4;3
SBY;;5;3
SDL;;4;3
SEG;;4;3
SHO;;4;3
SHY;;4;3
SILG;;3;3
SKL;;5;3
SKR;;4;3
SLD;;3;3
SMZG;;4;3
SNK;;4;3
SPSN;;3;3
SRO;;4;3
SSH;;6;4
SSLS;;4;3
SSWD;;4;3
STM;;4;3
STR;;5;3
SVL;;4;3
SWIH;;5;3
SZW;;4;3
TA;;4;3
TAE;;3;3
TAU;;5;4
TB;;5;3
TBF;;4;3
TBI;;3;3
TBM;;5;3
TBO;;4;3
TBSC;;4;3
TC;;4;3
TE;;4;3
TEL;;5;3
TET;;4;3
TF;;4;3
TFS;;3;3
TG;;4;3
TGAW;;3;3
TGL;;4;3
TGO;;5;3
TH;;5;3
THB;;4;3
THCH;;4;3
THD;;4;3
THE;;4;3
THF;;3;3
THT;;3;3
TKG;;4;3
TKH;;4;3
TKO;;4;3
TKT;;4;3
TL;;4;3
TLU;;5;3
TLW;;4;3
TM;;4;3
TMB;;5;3
TME;;4;3
TMG;;3;3
TNU;;4;3
TO;;4;3
TOE;;4;3
TP;;5;3
TPH;;4;3
TR;;5;3
TRE;;4;3
TS;TST;13;-
TRX;;4;3
TS;;8;4
TSC;;4;3
TSF;;5;3
TSFE;;3;3
TSG;;4;3
TSHT;;5;3
TSIG;;4;3
TSK;;3;3
TSRO;;3;3
TST;TS;13;-
TST;;3;-
TSU;;4;3
TSZ;;4;3
TRX;;4;-
TT;;5;3
TTR;;3;3
TTU;;5;3
TU;;7;4
TV;;5;3
TWD;;4;3
TWN;;5;3
UA;;4;3
UBK;;4;3
UE;;7;4
UEI;;5;4
UFT;;4;3
UG;;5;3
UGD;;4;3
UGH;;5;3
UGM;;4;-
UGO;;5;3
UGS;;3;3
UGW;;4;3
UHR;;3;3
UJP;;6;-
UKO;;4;-
UKU;;3;-
UL;;4;3
ULS;;4;3
UM;;4;3
UN;;5;3
UND;;4;3
UNM;;5;3
UO;;4;3
UPL;;4;3
URTB;;4;3
US;;5;3
USD;;5;-
USG;;4;3
USO;;5;3
UW;;4;3
UWE;;4;3
UWH;;4;3
UWK;;4;3
UWM;;6;4
UZL;;4;3
WA;;3;3
WB;;4;3
WBG;;5;3
WBR;;3;3
WE;;4;3
WG;;4;3
WHL;;4;3
WK;;5;3
WKA;;3;3
WL;;6;3
WLO;;3;3
WN;;3;3
WNRR;;4;3
WNS;;5;3
WNT;;4;3
WP;;5;3
WPM;;3;3
WPR;;3;3
WR;;6;4
WS;;6;3
WSR;;7;3
WV;;4;3
WW;;6;3
WWR;;4;3
WZS;;4;3
XAKN;;7;3
XASB;;8;4
XSS;;3;-
XTCH;;7;3)";

}  // hrd
}  // loader
}  // motis
