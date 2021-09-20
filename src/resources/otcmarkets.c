
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <csv.h>
#include <curl/curl.h>

#include "../config.h"
#include "../curl.h"
#include "../ticker-scraper.h"
#include "otcmarkets.h"

void explicit_bzero(void *s, size_t n);

const char *otcmarkets_industries[][2] = {
    { "Abrasive, Asbestos & Misc Nonmetallic Mineral Prods", "3290" },
    { "Accident and health insurance", "6321" },
    { "Adhesives and sealants", "2891" },
    { "Advertising agencies", "7311" },
    { "Advertising, misc", "7319" },
    { "Agricultural Chemicals", "2870" },
    { "Agricultural chemicals, misc", "2879" },
    { "Agricultural Prod-Livestock & Animal Specialties", "200" },
    { "Agricultural Production-Crops", "100" },
    { "Agricultural Services", "700" },
    { "Air courier services", "4513" },
    { "Air transportation, nonscheduled", "4522" },
    { "Air transportation, scheduled", "4512" },
    { "Air, water, and solid waste management", "9511" },
    { "Aircraft", "3721" },
    { "Aircraft engines and engine parts", "3724" },
    { "Aircraft parts and equipment, misc", "3728" },
    { "Airports, flying fields and services", "4581" },
    { "American Depositary Receipts", "8880" },
    { "Amusement and recreation, misc", "7999" },
    { "Analytical instruments", "3826" },
    { "Animal specialties, misc", "279" },
    { "Animal specialty services", "752" },
    { "Apartment building operators", "6513" },
    { "Apparel & Other Finishd Prods of Fabrics & Similar Matl", "2300" },
    { "Apparel and accessories, misc", "2389" },
    { "Asset managers", "6727" },
    { "Asset-Backed Securities", "6189" },
    { "Auto and home supply stores", "5531" },
    { "Automobiles and other vehicles", "5012" },
    { "Automotive services, misc", "7549" },
    { "Automotive stampings", "3465" },
    { "Bags; plastic, laminated, and coated", "2673" },
    { "Bakery Products", "2050" },
    { "Ball and roller bearings", "3562" },
    { "Bank holding companies", "6712" },
    { "Beef cattle feedlots", "211" },
    { "Beer and ale", "5181" },
    { "Berry crops", "171" },
    { "Beverages", "2080" },
    { "Biological products, except diagnostic", "2836" },
    { "Bituminous Coal & Lignite Mining", "1220" },
    { "Bituminous coal and lignite-surface", "1221" },
    { "Bituminous coal, underground", "1222" },
    { "Blank Checks", "6770" },
    { "Blast furnaces and steel mills", "3312" },
    { "Blowers and fans", "3564" },
    { "Boat building and repairing", "3732" },
    { "Bolts, nuts, rivets, and washers", "3452" },
    { "Book publishing", "2731" },
    { "Books, periodicals, and newspapers", "5192" },
    { "Bottled and canned soft drinks", "2086" },
    { "Brick, stone, and related material", "5032" },
    { "Bridge, tunnel, and elevated highway", "1622" },
    { "Broadwoven fabric mills, cotton", "2211" },
    { "Business associations", "8611" },
    { "Business consulting, misc", "8748" },
    { "Business services, misc", "7389" },
    { "Cable and pay television services", "4841" },
    { "Calculating and accounting equipment", "3578" },
    { "Cane sugar refining", "2062" },
    { "Canned fruits and specialities", "2033" },
    { "Canned specialties", "2032" },
    { "Canned, Frozen & Preservd Fruit, Veg & Food Specialties", "2030" },
    { "Carbon and graphite products", "3624" },
    { "Carwashes", "7542" },
    { "Catalog and mail-order houses", "5961" },
    { "Cement, hydraulic", "3241" },
    { "Cheese; natural and processed", "2022" },
    { "Chemical and fertilizer mining", "1479" },
    { "Chemical preparations, misc", "2899" },
    { "Chemicals & Allied Products", "2800" },
    { "Chemicals and allied products, misc", "5169" },
    { "Cigarettes", "2111" },
    { "Coal and other minerals and ores", "5052" },
    { "Coal mining services", "1241" },
    { "Coated fabrics, not rubberized", "2295" },
    { "Coating, Engraving & Allied Services", "3470" },
    { "Cogeneration Services & Small Power Producers", "4991" },
    { "Coin-operated amusement devices", "7993" },
    { "Colleges and universities", "8221" },
    { "Combination Utilities, misc", "4939" },
    { "Commercial banks, misc", "6029" },
    { "Commercial equipment, misc", "5046" },
    { "Commercial laundry equipment", "3582" },
    { "Commercial nonphysical research", "8732" },
    { "Commercial physical research", "8731" },
    { "Commercial Printing", "2750" },
    { "Commercial printing, lithographic", "2752" },
    { "Commercial printing, misc", "2759" },
    { "Commodity contracts brokers and dealers", "6221" },
    { "Communication services, misc", "4899" },
    { "Communications equipment, misc", "3669" },
    { "Computer & Office Equipment", "3570" },
    { "Computer and software stores", "5734" },
    { "Computer Communications Equipment", "3576" },
    { "Computer facilities management", "7376" },
    { "Computer integrated systems design", "7373" },
    { "Computer peripheral equipment, misc", "3577" },
    { "Computer related services, misc", "7379" },
    { "Computer rental and leasing", "7377" },
    { "Computer storage device", "3572" },
    { "Computer terminals", "3575" },
    { "Computers, peripherals, and software", "5045" },
    { "Concrete block and brick", "3271" },
    { "Concrete products, misc", "3272" },
    { "Concrete, Gypsum & Plaster Products", "3270" },
    { "Construction - Special Trade Contractors", "1700" },
    { "Construction and mining machinery", "5082" },
    { "Construction machinery", "3531" },
    { "Construction materials, misc", "5039" },
    { "Construction sand and gravel", "1442" },
    { "Construction, Mining & Materials Handling Machinery & Equip", "3530" },
    { "Converted Paper & Paperboard Prods (No Contaners/Boxes)", "2670" },
    { "Copper ores", "1021" },
    { "Cordage and twine", "2298" },
    { "Corn", "115" },
    { "Cotton", "131" },
    { "Credit reporting services", "7323" },
    { "Crop planting and protection", "721" },
    { "Crude petroleum and natural gas", "1311" },
    { "Crushed and broken limestone", "1422" },
    { "Current-carrying wiring services", "3643" },
    { "Custom compound purchased resins", "3087" },
    { "Custom computer programming services", "7371" },
    { "Cut stone and stone products", "3281" },
    { "Cutlery, Handtools & General Hardware", "3420" },
    { "Dairy farms", "241" },
    { "Dairy Products", "2020" },
    { "Data processing and preparation", "7374" },
    { "Deep sea foreign transport of freight", "4412" },
    { "Deep sea passenger transport, ex freight", "4481" },
    { "Dental equipment and supplies", "3843" },
    { "Department stores", "5311" },
    { "Detective and armored car service", "7381" },
    { "Diagnostic substances", "2835" },
    { "Direct mail advertising services", "7331" },
    { "Direct selling establishments", "5963" },
    { "Distilled and blended liquors", "2085" },
    { "Dog and cat food", "2047" },
    { "Dolls and stuffed toys", "3942" },
    { "Drilling oil and gas wells", "1381" },
    { "Drinking places", "5813" },
    { "Drugs, proprietaries, and sundries", "5122" },
    { "Drugstores and Proprietary Stores", "5912" },
    { "Dry, condensed, evaporated products", "2023" },
    { "Durable goods, misc", "5099" },
    { "Eating places", "5812" },
    { "Electric and other services combined", "4931" },
    { "Electric housewares and fans", "3634" },
    { "Electric Lighting & Wiring Equipment", "3640" },
    { "Electric services", "4911" },
    { "Electric, Gas & Sanitary Services", "4900" },
    { "Electrical apparatus and equipment", "5063" },
    { "Electrical appl., television, and radio", "5064" },
    { "Electrical equipment and supplies, misc", "3699" },
    { "Electrical Industrial Apparatus", "3620" },
    { "Electrical industrial apparatus", "3629" },
    { "Electrical lamps", "3641" },
    { "Electrical repair shops", "7629" },
    { "Electrical work", "1731" },
    { "Electromedical equipment", "3845" },
    { "Electrometallurgical products", "3313" },
    { "Electron tubes", "3671" },
    { "Electronic & Other Electrical Equipment (No Computer Equip)", "3600" },
    { "Electronic Components & Accessories", "3670" },
    { "Electronic components, misc", "3679" },
    { "Electronic computers", "3571" },
    { "Electronic connectors", "3678" },
    { "Electronic parts and equipment, misc", "5065" },
    { "Employment agencies", "7361" },
    { "Engineering services", "8711" },
    { "Engines & Turbines", "3510" },
    { "Entertainers and entertainment groups", "7929" },
    { "Environmental controls", "3822" },
    { "Equipment rental and leasing, misc", "7359" },
    { "Explosives", "2892" },
    { "Fabricated metal products, misc", "3499" },
    { "Fabricated plate work (boiler shop)", "3443" },
    { "Fabricated Rubber Products, NEC", "3060" },
    { "Fabricated structural metal", "3441" },
    { "Fabricated Structural Metal Products", "3440" },
    { "Facilities support services", "8744" },
    { "Family clothing stores", "5651" },
    { "Farm machinery and equipment", "3523" },
    { "Farm-product raw materials, misc", "5159" },
    { "Fats & Oils", "2070" },
    { "Federal and federally sponsored credit", "6111" },
    { "Federal reserve banks", "6011" },
    { "Federal savings institutions", "6035" },
    { "Ferro-alloy ores (except vanadium)", "1061" },
    { "Fertilizers, mixing only", "2875" },
    { "Finance Lessors", "6172" },
    { "Finance Services", "6199" },
    { "Fire protection", "9224" },
    { "Fire, marine, and casualty insurance", "6331" },
    { "Fish and seafoods", "5146" },
    { "Fishing, Hunting and Trapping", "900" },
    { "Flat glass", "3211" },
    { "Flavoring extracts and syrups, misc", "2087" },
    { "Flour and other grain mill products", "2041" },
    { "Food and Kindred Products", "2000" },
    { "Food crops grown under cover", "182" },
    { "Food preparations, misc", "2099" },
    { "Footwear", "5139" },
    { "Footwear, (No Rubber)", "3140" },
    { "Foreign bank and branches and agencies", "6081" },
    { "Foreign trade and international banks", "6082" },
    { "Forest products", "831" },
    { "Forestry", "800" },
    { "Freight transportation arranging", "4731" },
    { "Fresh and frozen packaged fish", "2092" },
    { "Frozen bakery products, except bread", "2053" },
    { "Frozen specialties, misc", "2038" },
    { "Fuel dealers, misc", "5989" },
    { "Fuel oil dealers", "5983" },
    { "Functions related to deposit banking", "6099" },
    { "Furniture and fixtures, misc", "2599" },
    { "Furniture stores", "5712" },
    { "Games, toys and children's vehicles", "3944" },
    { "Gas and other services combined", "4932" },
    { "Gas production and/or distribution", "4925" },
    { "Gas transmission and distribution", "4923" },
    { "Gaskets, Packg & Sealg Devices & Rubber & Plastics Hose", "3050" },
    { "General automotive repair shops", "1540" },
    { "General Bldg Contractors - Nonresidential Bldgs", "7538" },
    { "General Bldg Contractors - Residential Bldgs", "1520" },
    { "General farms, primarily crop", "191" },
    { "General industrial machinery", "3569" },
    { "General Industrial Machinery & Equipment", "3560" },
    { "General medical and surgical hospitals", "8062" },
    { "Glass & Glassware, Pressed or Blown", "3220" },
    { "Glass and glazing work", "1793" },
    { "Gold and Silver Ores", "1040" },
    { "Gold ores", "1041" },
    { "Grain Mill Products", "2040" },
    { "Grapes", "172" },
    { "Groceries and related products, misc", "5149" },
    { "Groceries, general line", "5141" },
    { "Grocery stores", "5411" },
    { "Guided Missiles & Space Vehicles & Parts", "3760" },
    { "Hand and edge tools, misc", "3423" },
    { "Hardware", "5072" },
    { "Hardware stores", "5251" },
    { "Hardware, misc", "3429" },
    { "Hazardous Waste Management", "4955" },
    { "Health and allied services, misc", "8099" },
    { "Heating equipment, except electric", "3433" },
    { "Heavy Construction Other Than Bldg Const - Contractors", "1600" },
    { "Heavy construction, misc", "1629" },
    { "Help supply services", "7363" },
    { "Highway and street construction", "1611" },
    { "Holding companies, misc", "6719" },
    { "Home furnishings", "5023" },
    { "Home health care services", "8082" },
    { "Hospital and medical service plans", "6324" },
    { "Hotels and motels", "7011" },
    { "Hotels, Rooming Houses, Camps & Other Lodging Places", "7000" },
    { "Household Appliances", "3630" },
    { "Household appliances, misc", "3639" },
    { "Household audio and video equipment", "3651" },
    { "Household furnishings, misc", "2392" },
    { "Household Furniture", "2510" },
    { "Ice cream and frozen desserts", "2024" },
    { "Industrial buildings and warehouses", "1541" },
    { "Industrial furnaces and ovens", "3567" },
    { "Industrial gases", "2813" },
    { "Industrial Inorganic Chemicals", "2810" },
    { "Industrial inorganic chemicals, misc", "2819" },
    { "Industrial machinery and equipment", "5084" },
    { "Industrial machinery, misc", "3599" },
    { "Industrial Organic Chemicals", "2860" },
    { "Industrial organic chemicals, misc", "2869" },
    { "Industrial supplies", "5085" },
    { "Industrial trucks and tractors", "3537" },
    { "Information retrieval", "7375" },
    { "Inorganic pigments", "2816" },
    { "Inspection and fixed facilities", "4785" },
    { "Instruments to measure electricity", "3825" },
    { "Insurance agents, brokers, & services", "6411" },
    { "Insurance carriers, misc", "6399" },
    { "Internal combustion engines, misc", "3519" },
    { "International affairs", "9721" },
    { "Investment advice", "6282" },
    { "Investment offices, misc", "6726" },
    { "Investors, misc", "6799" },
    { "Iron & Steel Foundries", "3320" },
    { "Iron ores", "1011" },
    { "Jewelers materials and lapidary work", "3915" },
    { "Jewelry and precious stones", "5094" },
    { "Jewelry stores", "5944" },
    { "Jewelry, precious metal", "3911" },
    { "Jewelry, Silverware & Plated Ware", "3910" },
    { "Laboratory apparatus and furniture", "3821" },
    { "Landscape counseling and planning", "781" },
    { "Laundry and garment services", "7219" },
    { "Lawn and garden equipment", "3524" },
    { "Lead and zinc ores", "1031" },
    { "Leather & Leather Products", "3100" },
    { "Lessors of Real Property, NEC", "6519" },
    { "Life insurance", "6311" },
    { "Lighting equipment, misc", "3648" },
    { "Loan brokers", "6163" },
    { "Local & Suburban Transit & Interurban Hwy Passenger Trans", "4100" },
    { "Local passenger transportation, misc", "4119" },
    { "Logging", "2411" },
    { "Lubricating oils and greases", "2992" },
    { "Lumber & Wood Products (No Furniture)", "2400" },
    { "Lumber and other building materials", "5211" },
    { "Lumber, plywood, and millwork", "5031" },
    { "Machine tool accessories", "3545" },
    { "Machine tools, metal cutting type", "3541" },
    { "Magnetic and optical recording media", "3695" },
    { "Malt beverages", "2082" },
    { "Management consulting services", "8742" },
    { "Management investment, open-ended", "6722" },
    { "Management services", "8741" },
    { "Manifold business forms", "2761" },
    { "Manufacturing industries, misc", "3999" },
    { "Marine cargo handling", "4491" },
    { "Measuring and controlling devices, misc", "3829" },
    { "Meat packing plants", "2011" },
    { "Meats and meat products", "5147" },
    { "Medical and hospital equipment", "5047" },
    { "Medical laboratories", "8071" },
    { "Medicinal and botanicals", "2833" },
    { "Membership organizations", "8699" },
    { "Membership sports and recreation clubs", "7997" },
    { "Men's & Boys' Furnishgs, Work Clothg, & Allied Garments", "2320" },
    { "Mens and boys clothing misc", "2329" },
    { "Mens and boys suits and coats", "2311" },
    { "Mens and boys work clothing", "2326" },
    { "Mens footwear, except athletic", "3143" },
    { "Merchandising machine operators", "5962" },
    { "Metal barrels, drums, and pails", "3412" },
    { "Metal doors, sash, and trim", "3442" },
    { "Metal Forgings & Stampings", "3460" },
    { "Metal Mining", "1000" },
    { "Metal mining services", "1081" },
    { "Metal ores, misc", "1099" },
    { "Metal stamping, misc", "3469" },
    { "Metals service centers and offices", "5051" },
    { "Metalworkg Machinery & Equipment", "3540" },
    { "Metalworking machinery, misc", "3549" },
    { "Millwood, Veneer, Plywood, & Structural Wood Members", "2430" },
    { "Mineral Royalty Traders", "6795" },
    { "Minerals, ground or treated", "3295" },
    { "Mining & Quarrying of Nonmetallic Minerals (No Fuels)", "1400" },
    { "Misc Industrial & Commercial Machinery & Equipment", "3590" },
    { "Miscellaneous business credit", "6159" },
    { "Miscellaneous Chemical Products", "2890" },
    { "Miscellaneous Electrical Machinery, Equipment & Supplies", "3690" },
    { "Miscellaneous Fabricated Metal Products", "3490" },
    { "Miscellaneous Fabricated Textile Products", "2390" },
    { "Miscellaneous Food Preparations & Kindred Products", "2090" },
    { "Miscellaneous food stores", "5499" },
    { "Miscellaneous Furniture & Fixtures", "2590" },
    { "Miscellaneous general merchandise", "5399" },
    { "Miscellaneous home furnishings", "5719" },
    { "Miscellaneous investing", "6790" },
    { "Miscellaneous Manufacturing Industries", "3990" },
    { "Miscellaneous marine products", "919" },
    { "Miscellaneous Metal Ores ", "1090" },
    { "Miscellaneous nonmetallic mining", "1499" },
    { "Miscellaneous personal services", "7299" },
    { "Miscellaneous Plastics Products", "3080" },
    { "Miscellaneous Primary Metal Products", "3390" },
    { "Miscellaneous Products of Petroleum & Coal", "2990" },
    { "Miscellaneous publishing", "2741" },
    { "Miscellaneous retail stores, misc", "5999" },
    { "Miscellaneous Transportation Equipment", "3790" },
    { "Mobile homes", "2451" },
    { "Mortgage bankers and correspondents", "6162" },
    { "Motion picture and tape distribution", "7822" },
    { "Motion picture and video production", "7812" },
    { "Motion picture distribution services", "7829" },
    { "Motor vehicle parts and accessories", "3714" },
    { "Motor vehicle supplies and new parts", "5013" },
    { "Motor vehicles and car bodies", "3711" },
    { "Motorcycles, bicycles, and parts", "3751" },
    { "Motors and generators", "3621" },
    { "Musical instruments", "3931" },
    { "National commercial banks", "6021" },
    { "National security", "9711" },
    { "Natural gas distribution", "4924" },
    { "Natural gas transmission", "4922" },
    { "Newspapers", "2711" },
    { "Nitrogenous fertilizers", "2873" },
    { "Non-Operating Establishments", "9995" },
    { "Nonclassifiable establishments", "9999" },
    { "Nondurable goods, misc", "5199" },
    { "Nonferrous Foundries (Castings)", "3360" },
    { "Nonferrous foundries, misc", "3369" },
    { "Nonferrous wire drawing and insulating", "3357" },
    { "Nonmetallic mineral services", "1481" },
    { "Nonresidential building operators", "6512" },
    { "Nonresidential construction, misc", "1542" },
    { "Office furniture, except wood", "2522" },
    { "Offices and clinics of dentists", "8021" },
    { "Offices and clinics of medical doctors", "8011" },
    { "Offices health practitioners", "8049" },
    { "Oil and gas exploration services", "1382" },
    { "Oil and gas field machinery", "3533" },
    { "Oil and gas field services, misc", "1389" },
    { "Oil Royalty traders", "6792" },
    { "Operative builders", "1531" },
    { "Ophthalmic goods", "3851" },
    { "Optical instruments and lenses", "3827" },
    { "Ordnance & Accessories, (No Vehicles/Guided Missiles)", "3480" },
    { "Ordnance and accessories, misc", "3489" },
    { "Ornamental shrub and tree services", "783" },
    { "Outdoor advertising services", "7312" },
    { "Packaged frozen foods", "5142" },
    { "Painting and paper hanging", "1721" },
    { "Paints and allied products", "2851" },
    { "Paper mills", "2621" },
    { "Paper; coated and laminated packaging", "2671" },
    { "Paperboard Containers & Boxes", "2650" },
    { "Papers & Allied Products", "2600" },
    { "Patent owners and lessors", "6794" },
    { "Pension, health, and welfare funds", "6371" },
    { "Periodicals", "2721" },
    { "Personal credit institutions", "6141" },
    { "Petroleum and coal products, misc", "2999" },
    { "Petroleum bulk stations and terminals", "5171" },
    { "Petroleum products, misc", "5172" },
    { "Petroleum refining", "2911" },
    { "Pharmaceutical preparations", "2834" },
    { "Photographic equipment and supplies", "3861" },
    { "Pipe Lines (No Natural Gas)", "4610" },
    { "Plastic Material, Synth Resin/Rubber, Cellulos (No Glass)", "2820" },
    { "Plastic materials and basic shapes", "5162" },
    { "Plastics foam products", "3086" },
    { "Plastics materials and resins", "2821" },
    { "Plastics products, misc", "3089" },
    { "Plumbing, heating, air-conditioning", "1711" },
    { "Polishes and sanitation goods", "2842" },
    { "Potash, soda, borate minerals", "1474" },
    { "Poultry slaughtering and processing", "2015" },
    { "Power Transmission equipment, misc", "3568" },
    { "Prefabricated wood buildings", "2452" },
    { "Prepackaged software", "7372" },
    { "Prerecorded records and tapes", "3652" },
    { "Pressed and blown glass, misc", "3229" },
    { "Primary aluminum", "3334" },
    { "Primary batteries, dry and wet", "3692" },
    { "Primary copper", "3331" },
    { "Primary metal products", "3399" },
    { "Primary nonferrous metals, misc", "3339" },
    { "Primary Smelting & Refining of Nonferrous Metals", "3330" },
    { "Printed circuit boards", "3672" },
    { "Printing ink", "2893" },
    { "Printing trades machinery", "3555" },
    { "Process control instruments", "3231" },
    { "Products of purchased glass", "3823" },
    { "Psychiatric hospitals", "8063" },
    { "Public building and related furniture", "2531" },
    { "Public Warehousing & Storage", "4220" },
    { "Pulp mills", "2611" },
    { "Pumps and pumping equipment", "3561" },
    { "Racing, including track operation", "7948" },
    { "Radio and TV communications equipment", "3663" },
    { "Radio broadcasting stations", "4832" },
    { "Radio, television, and electronics stores", "5731" },
    { "Radio, TV, publisher representative.", "7313" },
    { "Radiotelephone Communication", "4812" },
    { "Railroad equipment", "3743" },
    { "Railroads, line-haul operating", "4011" },
    { "Ready-mix concrete", "3273" },
    { "Real Estate", "6500" },
    { "Real estate agents and managers", "6531" },
    { "Real Estate Dealers (For Their Own Account)", "6532" },
    { "Real estate investment trusts", "6798" },
    { "Real Estate Operators (No Developers) & Lessors", "6510" },
    { "Refrigeration & Service Industry Machinery", "3580" },
    { "Refrigeration and heating equipment", "3585" },
    { "Refuse systems", "4953" },
    { "Residential construction, misc", "1522" },
    { "Retail nursery and garden stores", "5261" },
    { "Retail-Apparel & Accessory Stores", "5600" },
    { "Retail-Auto Dealers & Gasoline Stations", "5500" },
    { "Retail-Building Materials, Hardware, Garden Supply", "5200" },
    { "Retail-Convenience Stores", "5412" },
    { "Retail-Eating & Drinking Places", "5810" },
    { "Retail-Food Stores", "5400" },
    { "Retail-Home Furniture, Furnishings & Equipment Stores", "5700" },
    { "Retail-Miscellaneous Retail", "5900" },
    { "Retail-Miscellaneous Shopping Goods Stores", "5940" },
    { "Retail-Nonstore Retailers", "5960" },
    { "Retail-Retail Stores, NEC", "5990" },
    { "Rice", "112" },
    { "Rolling Drawing & Extruding of Nonferrous Metals", "3350" },
    { "Roofing, siding, and insulation", "5033" },
    { "Rubber and plastics footwear", "3021" },
    { "Salted and roasted nuts and seeds", "2068" },
    { "Sanitary Services", "4950" },
    { "Sanitary services, misc", "4959" },
    { "Sausages and other prepared meats", "2013" },
    { "Savings institutions, except federal", "6036" },
    { "Schools and educational services", "8299" },
    { "Scrap and waste materials", "5093" },
    { "Screw machine products", "3451" },
    { "Search and navigation equipment", "3812" },
    { "Secondary nonferrous metals", "3341" },
    { "Security & commodity brokers & dealers", "6231" },
    { "Security & Commodity Brokers, Dealers, Exchanges & Services", "6200" },
    { "Security and commodity service", "6289" },
    { "Security brokers and dealers", "6211" },
    { "Security systems services", "7382" },
    { "Semiconductors and related devices", "3674" },
    { "Service establishment equipment", "5087" },
    { "Service industry machinery, misc", "3589" },
    { "Services allied to motion pictures", "7819" },
    { "Services, misc", "8999" },
    { "Services-Advertising", "7310" },
    { "Services-Amusement & Recreation Services", "7900" },
    { "Services-Auto Rental & Leasing (No Drivers)", "7510" },
    { "Services-Automotive Repair, Services & Parking", "7500" },
    { "Services-Computer Programming, Data Processing, Etc.", "7370" },
    { "Services-Consumer Credit Reporting, Collection Agencies", "7320" },
    { "Services-Educational Services", "8200" },
    { "Services-Engineering, Accounting, Research, Management", "8700" },
    { "Services-Health Services", "8000" },
    { "Services-Hospitals", "8060" },
    { "Services-Mailing, Reproduction, Commercial Art & Photography", "7330" },
    { "Services-Misc Health & Allied Services, NEC", "8090" },
    { "Services-Miscellaneous Amusement & Recreation", "7990" },
    { "Services-Miscellaneous Business Services", "7380" },
    { "Services-Miscellaneous Equipment Rental & Leasing", "7350" },
    { "Services-Miscellaneous Repair Services", "7600" },
    { "Services-Motion Picture Theaters", "7830" },
    { "Services-Nursing & Personal Care Facilities", "8050" },
    { "Services-Personal Services", "7200" },
    { "Services-Services, NEC", "8900" },
    { "Services-Social Services", "8300" },
    { "Services-Telephone Interconnect Systems", "7385" },
    { "Services-To Dwellings & Other Buildings", "7340" },
    { "Sheet Metal Work", "3444" },
    { "Ship & Boat Building & Repairing", "3730" },
    { "Ship building and repairing", "3731" },
    { "Short-term business credit", "6153" },
    { "Signs and advertising specialties", "3993" },
    { "Silver ores", "1044" },
    { "Skilled nursing care facilities", "8051" },
    { "Soap and other detergents", "2841" },
    { "Soap, Detergents, Cleaning Preparations, Perfumes, Cosmetics", "2840" },
    { "Social services, misc", "8399" },
    { "Space vehicle equipment, misc", "3769" },
    { "Special Industry Machinery (No Metalworking Machinery)", "3550" },
    { "Special industry machinery, misc", "3559" },
    { "Special warehousing and storage", "4226" },
    { "Specialty hospitals, except psychiatric", "8069" },
    { "Specialty outpatient clinics, misc", "8093" },
    { "Sporting and athletic goods, misc", "3949" },
    { "Sporting and recreation goods", "5091" },
    { "Sporting goods and bicycle shops", "5941" },
    { "Sports clubs, managers, and promoters", "7941" },
    { "State commercial banks", "6022" },
    { "Steam and air-conditioning supply", "4961" },
    { "Steel foundries, misc", "3325" },
    { "Steel pipe and tubes", "3317" },
    { "Steel wire and related products", "3315" },
    { "Steel Works, Blast Furnaces & Rolling & Finishing Mills", "3310" },
    { "Storage batteries", "3691" },
    { "Structural clay products, misc", "3259" },
    { "Subdivisers and developers, misc", "6552" },
    { "Sugar & Confectionery Products", "2060" },
    { "Surety insurance", "6351" },
    { "Surgical and medical instruments", "3841" },
    { "Surgical appliances and supplies", "3842" },
    { "Switchgear and switchboard apparatus", "3613" },
    { "Telegraph and other communications", "4822" },
    { "Telephone and telegraph apparatus", "3661" },
    { "Telephone communication, except radio", "4813" },
    { "Television broadcasting stations", "4833" },
    { "Testing laboratories", "8734" },
    { "Textile Mill Products", "2200" },
    { "Theatrical producers and services", "7922" },
    { "Timber tracts", "811" },
    { "Tires and inner tubes", "3011" },
    { "Tires and tubes", "5014" },
    { "Title insurance", "6361" },
    { "Tobacco Products", "2100" },
    { "Tobacco stores and stands", "5993" },
    { "Toilet preparations", "2844" },
    { "Tour operators", "4725" },
    { "Toys and hobby goods and supplies", "5092" },
    { "Transformers, except electric", "3612" },
    { "Transportation equipment and supplies", "5088" },
    { "Transportation equipment, misc", "3799" },
    { "Transportation Services", "4700" },
    { "Transportation services, misc", "4789" },
    { "Travel agencies", "4724" },
    { "Travel trailers and campers", "3792" },
    { "Trucking & Courier Services (No Air)", "4210" },
    { "Trucking terminal facilities", "4231" },
    { "Trucking, except local", "4213" },
    { "Trusts, misc", "6733" },
    { "Turbines and turbine generator sets", "3511" },
    { "Unsupported plastics film and sheet", "3081" },
    { "Uranium-radium-vanadium ores", "1094" },
    { "Variety stores", "5331" },
    { "Vegetables and melons", "161" },
    { "Video-tape rental", "7841" },
    { "Vitreous china table and kitchenware", "3262" },
    { "Watches, clocks, watch cases and parts", "3873" },
    { "Water supply", "4941" },
    { "Water Transportation", "4400" },
    { "Water, sewer, and utility lines", "1623" },
    { "Welding apparatus", "3548" },
    { "Wholesale-Apparel, Piece Goods & Notions", "5130" },
    { "Wholesale-Beer, Wine & Distilled Alcoholic Beverages", "5180" },
    { "Wholesale-Chemicals & Allied Products", "5160" },
    { "Wholesale-Durable Goods", "5000" },
    { "Wholesale-Farm Product Raw Materials", "5150" },
    { "Wholesale-Furniture & Home Furnishings", "5020" },
    { "Wholesale-Groceries & Related Products", "5140" },
    { "Wholesale-Hardware & Plumbing & Heating Equipment & Supplies", "5070" },
    { "Wholesale-Lumber & Other Construction Materials", "5030" },
    { "Wholesale-Machinery, Equipment & Supplies", "5080" },
    { "Wholesale-Metals & Minerals (No Petroleum)", "5050" },
    { "Wholesale-Misc Durable Goods", "5090" },
    { "Wholesale-Miscellaneous Nondurable Goods", "5190" },
    { "Wholesale-Motor Vehicles & Motor Vehicle Parts & Supplies", "5010" },
    { "Wholesale-Paper & Paper Products", "5110" },
    { "Wholesale-Professional & Commercial Equipment & Supplies", "5040" },
    { "Wine and distilled beverages", "5182" },
    { "Wines, brandy, and brandy spirits", "2084" },
    { "Women's, Misses', Children's & Infants' Undergarments", "2340" },
    { "Womens and childrens clothing", "5137" },
    { "Womens and childrens underwear", "2341" },
    { "Womens and misses blouses and shirts", "2331" },
    { "Womens clothing stores", "5621" },
    { "Womens footwear, except athletic", "3144" },
    { "Womens hosiery, except socks", "2251" },
    { "Wood household furniture", "2511" },
    { "Wood pallets and skids", "2448" },
    { "Wood partition and fixtures", "2541" },
    { "Wood products, misc", "2499" },
    { "Woodworking machinery", "3553" },
    { "X-ray apparatus and tubes", "3844" }
};

char otcmarkets_markets_otcqb[] = {
    10 // OTCQB
};

char otcmarkets_markets_otcqx[] = {
    6, // OTCQX International
    5, // OTCQX International Premier
    2, // UTCQX U.S.
    1  // UTCQX U.S. Premier
};

char otcmarkets_markets_pink[] = {
    20, // Pink Current
    21, // Pink Limited
    22  // Pink No Information
};

CURL *curl_handle;
int otcmarkets_csv_row_index = 0;
int otcmarkets_csv_field_index = 0;
DataRow otc_markets_data_row;

void otcmarkets_csv_cb_end_of_field(void *s, size_t i, void *outfile) {
    (void)(outfile); // Suppress "unused parameter" compiler warning

    if (otcmarkets_csv_row_index > 0) {
        int field_value_len = i + 1;
        char string[field_value_len];
        explicit_bzero(string, sizeof(string));
        memcpy(&string, s, i);
        string[field_value_len] = '\0';

        switch (otcmarkets_csv_field_index) {
            case 0:
                memcpy(otc_markets_data_row.ticker, string, field_value_len);
                break;
            case 1:
                memcpy(otc_markets_data_row.company, string, field_value_len);
                break;
            case 3:
                memcpy(otc_markets_data_row.price, string, field_value_len);
                break;
            case 7:
                memcpy(otc_markets_data_row.country, string, field_value_len);
                break;
        }
    }

    otcmarkets_csv_field_index++;
}

void otcmarkets_csv_cb_end_of_row(int c, void *outfile) {
    (void)(c); // Suppress "unused parameter" compiler warning
    (void)(outfile); // Suppress "unused parameter" compiler warning

    if (otcmarkets_csv_row_index > 0) {
        ticker_scraper_add(&otc_markets_data_row);
    }

    otcmarkets_csv_field_index = 0;
    otcmarkets_csv_row_index++;
}

int otcmarkets_parse_data_from_csv(struct MemoryStruct *chunk, const MarketPlace marketplace, const char *industry[])
{
    struct csv_parser parser;
    unsigned char csv_options = CSV_STRICT;
    int new = 0;

    // TODO: reset otc_markets_data_row here

#if DEBUG
    fprintf(stderr, "Parsing CSV file for industry %s of marketplace %s\n", industry[0], marketplace_to_str(marketplace));
#endif

    otc_markets_data_row.marketplace = marketplace;

    strncpy(otc_markets_data_row.industry, industry[0], sizeof(otc_markets_data_row.industry) - 1);

    /* Sector data is not provided by otcmarkets.com */
    explicit_bzero(otc_markets_data_row.sector, sizeof(otc_markets_data_row.sector));

    if (csv_init(&parser, csv_options) != 0) {
        fprintf(stderr, "Error: Couldn’t initialize CSV parser\n");
        exit(EXIT_FAILURE);
    }
    csv_set_delim(&parser, CSV_COMMA);

    otcmarkets_csv_row_index = 0;

    csv_parse(&parser, chunk->memory, strlen(chunk->memory), otcmarkets_csv_cb_end_of_field, otcmarkets_csv_cb_end_of_row, stdout);

    csv_free(&parser);

    return new;
}

int otcmarkets_retrieve_csv_file_for_industry(struct MemoryStruct *chunk, const MarketPlace marketplace, const char *industry[])
{
    CURLcode res;

    chunk->memory = malloc(1); /* Will be grown as needed by realloc() in nerd_trader_curl_write_memory_callback */
    chunk->size = 0; /* No data at this point */

    char url[256] = "https://www.otcmarkets.com/research/stock-screener/api/downloadCSV";

    int retrieved = 0;

    strcat(url, "?securityType=");
    strcat(url, "Common%20Stock");

    strcat(url, "&industry=");
    strcat(url, industry[1]);

    strcat(url, "&market=");
    /* Construct market URL query parameter */
    {
        int i = 0;
        char markets[5] = { 0, 0, 0, 0, 0};
        char n[6] = "";

        switch (marketplace)
        {
            case OTCQX:
                memcpy(markets, otcmarkets_markets_otcqx, sizeof(otcmarkets_markets_otcqx));
            break;

            case OTCQB:
                memcpy(markets, otcmarkets_markets_otcqb, sizeof(otcmarkets_markets_otcqb));
            break;

            case PINK:
                memcpy(markets, otcmarkets_markets_pink, sizeof(otcmarkets_markets_pink));
            break;

            default:
                exit(EXIT_FAILURE);
        }

        while (true) {
            /* Put comma in front of value (unless the result string is empty) */
            if (i > 0) {
                strcat(url, ",");
            }

            sprintf(n, "%d", markets[i]);
            strcat(url, n);

            i++;

            if (markets[i] == 0) {
                break;
            }
        }
    }

    curl_easy_setopt(curl_handle, CURLOPT_URL, url);

#if DEBUG
    fprintf(stderr, "Attempting to obtain CSV file for industry \"%s\" of marketplace %s: %s\n", industry[0], marketplace_to_str(marketplace), url);
#endif

    /* Get it! */
    res = curl_easy_perform(curl_handle);

    /* Check for errors */
    if (res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        retrieved = otcmarkets_parse_data_from_csv(chunk, marketplace, industry);
    }

    return retrieved;
}

int ticker_scraper_scrape_otcmarkets(const MarketPlace marketplace)
{
    struct MemoryStruct chunk;
    int new = 0;

    /* Init curl */
    curl_handle = nerd_trader_curl_init(&chunk);

    /* Iterate over industries, splitting */
    for (int i = 0, ilen = sizeof(otcmarkets_industries) / sizeof(otcmarkets_industries[0]); i < ilen; i++) {
        new += otcmarkets_retrieve_csv_file_for_industry(&chunk, marketplace, otcmarkets_industries[i]);
    }

    nerd_trader_curl_cleanup(curl_handle);

    return new;
}
