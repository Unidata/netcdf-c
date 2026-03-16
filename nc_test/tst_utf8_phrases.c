
/*
 * Copyright 1998-2026 University Corporation for Atmospheric Research/Unidata
 *  See the LICENSE file for more information.
 */

#include <config.h>
#include <stdlib.h>
#include <nc_tests.h>
#include <netcdf.h>
#include <string.h>

#include "ncutf8.h"

/*
  The test here are taken from the UTF-8 SAMPLER

    Frank da Cruz
    The Kermit Project <http://kermitproject.org/index.html>
    New York City
    fdc@kermitproject.org <mailto:fdc@kermitproject.org>

    /Last update:/ Tue Jan 31 16:56:13 2017
*/


struct Test {
    int xfail;
    const char* id;
    const char* description;
    const char* data;
};
#define NULLTEST {0,NULL,NULL,NULL}

static const struct Test utf8currency[] = {
{0,"1.1","Currencies",  "¥£€$¢₡₢₣₤₥₦₧₨₩₪₫₭₮₯₹"},
NULLTEST
};

static const struct Test utf8poems[] = {
{0,"2.1","Runes",
"ᚠᛇᚻ᛫ᛒᛦᚦ᛫ᚠᚱᚩᚠᚢᚱ᛫ᚠᛁᚱᚪ᛫ᚷᛖᚻᚹᛦᛚᚳᚢᛗ\nᛋᚳᛖᚪᛚ᛫ᚦᛖᚪᚻ᛫ᛗᚪᚾᚾᚪ᛫ᚷᛖᚻᚹᛦᛚᚳ᛫ᛗᛁᚳᛚᚢᚾ᛫ᚻᛦᛏ᛫ᛞᚫᛚᚪᚾ\nᚷᛁᚠ᛫ᚻᛖ᛫ᚹᛁᛚᛖ᛫ᚠᚩᚱ᛫ᛞᚱᛁᚻᛏᚾᛖ᛫ᛞᚩᛗᛖᛋ᛫ᚻᛚᛇᛏᚪᚾ᛬\n"
},
{0,"2.2","Middle English",
    "An preost wes on leoden, Laȝamon was ihoten"
    "He wes Leovenaðes sone -- liðe him be Drihten."
    "He wonede at Ernleȝe at æðelen are chirechen,"
    "Uppen Sevarne staþe, sel þar him þuhte,"
    "Onfest Radestone, þer he bock radde."
},
{0,"2.3","Middle High German",
"Sîne klâwen durh die wolken sint geslagen,"
"er stîget ûf mit grôzer kraft,"
"ich sih in grâwen tägelîch als er wil tagen,"
"den tac, der im geselleschaft"
"erwenden wil, dem werden man,"
"den ich mit sorgen în verliez."
"ich bringe in hinnen, ob ich kan."
"sîn vil manegiu tugent michz leisten hiez."
},

{0,"2.4",
"Greek.1",
"Τη γλώσσα μου έδωσαν ελληνικ το σπίτι φτωχικό στις αμμουδιές του Ομήρου. Μονάχη έγνοια η γλώσσα μου στις αμμουδιές του Ομήρου. από το Άξιον Εστί του Οδυσσέα Ελύτη"
},
{0,"2.5",
"Greek.2",
"Τὴ γλῶσσα μοῦ ἔδωσαν ἑλληνικὴτὸ σπίτι φτωχικὸ στὶς ἀμμουδιὲς τοῦ Ὁμήρου. Μονάχη ἔγνοια ἡ γλῶσσα μου στὶς ἀμμουδιὲς τοῦ Ὁμήρου.ἀπὸ τὸ Ἄξιον ἐστί τοῦ Ὀδυσσέα Ἐλύτη"
},
{0,"2.6",
"Russian",
"На берегу пустынных волнСтоял он, дум великих полн,И вдаль глядел. Пред ним широкоРека неслася; бедный чёлнПо ней стремился одиноко.По мшистым, топким берегамЧернели избы здесь и там,Приют убогого чухонца;И лес, неведомый лучамВ тумане спрятанного солнца, Кругом шумел."
},
{0,"2.7",
"Georgian",
"ვეპხის ტყაოსანი შოთა რუსთაველიღმერთსი შემვედრე, ნუთუ კვლა დამხსნას სოფლისა შრომასა, ცეცხლს, წყალსადა მიწასა, ჰაერთა თანა მრომასა; მომცნეს ფრთენი და აღვფრინდე,მივჰხვდე მას ჩემსა ნდომასა, დღისით და ღამით ვჰხედვიდე მზისა ელვათა კრთომაასა."
},
{0,"2.8",
"Tamil.1",
"யாமறிந்த மொழிகளிலே தமிழ்மொழி போல் இனிதாவது எங்கும் காணோம்,பாமரராய் விலங்குகளாய், உலகனைத்தும் இகழ்ச்சிசொலப் பான்மை கெட்டு,நாமமது தமிழரெனக் கொண்டு இங்கு வாழ்ந்திடுதல் நன்றோ? சொல்லீர்! தேமதுரத் தமிழோசை உலகமெலாம் பரவும்வகை செய்தல் வேண்டும்."
},
{0,"2.9",
"Tamil.2",
"ಬಾ ಇಲ್ಲಿ ಸಂಭವಿಸು ಇಂದೆನ್ನ ಹೃದಯದಲಿನಿತ್ಯವೂ ಅವತರಿಪ ಸತ್ಯಾವತಾರಮಣ್ಣಾಗಿ ಮರವಾಗಿ ಮಿಗವಾಗಿ ಕಗವಾಗೀ...ಮಣ್ಣಾಗಿ ಮರವಾಗಿ ಮಿಗವಾಗಿ ಕಗವಾಗಿಭವ ಭವದಿ ಭತಿಸಿಹೇ ಭವತಿ ದೂರ ನಿತ್ಯವೂ ಅವತರಿಪ ಸತ್ಯಾವತಾರ || ಬಾ ಇಲ್ಲಿ ||"
},
NULLTEST
};

static const struct Test utf8phrases1[] = {
{0,"3.1","Sanskrit", "﻿काचं शक्नोम्यत्तुम् । नोपहिनस्ति माम् ॥"},
{0,"3.2","Sanskrit/(standard transcription)", "kācaṃ śaknomyattum nopahinasti mām."},
{0,"3.3","Classical Greek", "ὕαλον ϕαγεῖν δύναμαι· τοῦτο οὔ με βλάπτει."},
{0,"3.4","Greek (monotonic)", "Μπορώ να φάω σπασμένα γυαλιά χωρίς να πάθω τίποτα."},
{0,"3.5","Greek (polytonic)", "Μπορῶ νὰ φάω σπασμένα γυαλιὰ χωρὶς νὰ πάθω τίποτα."},
{0,"3.6","Latin", "Vitrum edere possum; mihi non nocet."},
{0,"3.7","Old French", "Je puis mangier del voirre. Ne me nuit."},
{0,"3.8","French", "Je peux manger du verre, ça ne me fait pas mal."},
{0,"3.9","Provençal / Occitan", "Pòdi manjar de veire, me nafrariá pas."},
{0,"3.10","Québécois", "J'peux manger d'la vitre, ça m'fa pas mal."},
{0,"3.11","Walloon", "Dji pou magnî do vêre, çoula m' freut nén må."},
{0,"3.12","Picard", "Ch'peux mingi du verre, cha m'foé mie n'ma."},
{0,"3.13","Kreyòl Ayisyen (Haitï)", "Mwen kap manje vè, li pa blese'm."},
{0,"3.14","Basque", "Kristala jan dezaket, ez dit minik ematen."},
{0,"3.15","Catalan / Català", "Puc menjar vidre, que no em fa mal."},
{0,"3.16","Spanish", "Puedo comer vidrio, no me hace daño."},
{0,"3.17","Aragonés", "Puedo minchar beire, no me'n fa mal ."},
{0,"3.18","Galician", "Eu podo xantar cristais e non cortarme."},
{0,"3.19","European Portuguese", "Posso comer vidro, não me faz mal."},
{0,"3.20","Brazilian Portuguese (8 <#notes>)", "Posso comer vidro, não me machuca."},
{0,"3.21","Caboverdiano/Kabuverdianu (Cape Verde)", "M' podê cumê vidru, ca ta maguâ-m'."},
{0,"3.22","Papiamentu", "Ami por kome glas anto e no ta hasimi daño."},
{0,"3.23","Italian", "Posso mangiare il vetro e non mi fa male."},
{0,"3.24","Milanese", "Sôn bôn de magnà el véder, el me fa minga mal."},
{0,"3.25","Roman", "Me posso magna' er vetro, e nun me fa male."},
{0,"3.26","Napoletano", "M' pozz magna' o'vetr, e nun m' fa mal."},
{0,"3.27","Venetian", "Mi posso magnare el vetro, no'l me fa mae."},
{0,"3.28","Zeneise /(Genovese)", "/ Pòsso mangiâ o veddro e o no me fà mâ."},
{0,"3.29","Sicilian", "Puotsu mangiari u vitru, nun mi fa mali."},
{0,"3.30","Romansch (Grischun)", "Jau sai mangiar vaider, senza che quai fa donn a mai."},
{0,"3.31","Romanian", "Pot să mănânc sticlă și ea nu mă rănește."},
{0,"3.32","Esperanto", "Mi povas manĝi vitron, ĝi ne damaĝas min."},
{0,"3.33","Cornish", "Mý a yl dybry gwéder hag éf ny wra ow ankenya."},
{0,"3.34","Welsh", "Dw i'n gallu bwyta gwydr, 'dyw e ddim yn gwneud dolur i mi."},
{0,"3.35","Manx Gaelic", "Foddym gee glonney agh cha jean eh gortaghey mee."},
{0,"3.36","Old Irish /(Ogham)", "/ ᚛᚛ᚉᚑᚅᚔᚉᚉᚔᚋ ᚔᚈᚔ ᚍᚂᚐᚅᚑ ᚅᚔᚋᚌᚓᚅᚐ᚜"},
{0,"3.37","Old Irish /(Latin)", "/ Con·iccim ithi nglano. Ním·géna."},
{0,"3.38","Irish", "Is féidir liom gloinne a ithe. Ní dhéanann sí dochar ar bith dom."},
{0,"3.39","Ulster Gaelic", "Ithim-sa gloine agus ní miste damh é."},
{0,"3.40","Scottish Gaelic", "S urrainn dhomh gloinne ithe; cha ghoirtich i mi."},
{0,"3.41","Anglo-Saxon /(Runes)", "/ ᛁᚳ᛫ᛗᚨᚷ᛫ᚷᛚᚨᛋ᛫ᛖᚩᛏᚪᚾ᛫ᚩᚾᛞ᛫ᚻᛁᛏ᛫ᚾᛖ᛫ᚻᛖᚪᚱᛗᛁᚪᚧ᛫ᛗᛖ᛬"},
{0,"3.42","Anglo-Saxon /(Latin)", "/ Ic mæg glæs eotan ond hit ne hearmiað me."},
{0,"3.43","Middle English", "Ich canne glas eten and hit hirtiþ me nouȝt."},
{0,"3.44","English", "I can eat glass and it doesn't hurt me."},
{0,"3.45","English /(IPA)", "/ [aɪ kæn iːt glɑːs ænd ɪt dɐz nɒt hɜːt miː]"},
{0,"3.46","English /(Braille)", "/ ⠊⠀⠉⠁⠝⠀⠑⠁⠞⠀⠛⠇⠁⠎⠎⠀⠁⠝⠙⠀⠊⠞⠀⠙⠕⠑⠎⠝⠞⠀⠓⠥⠗⠞⠀⠍⠑"},
{0,"3.47","Jamaican", "Mi kian niam glas han i neba hot mi."},
{0,"3.48","Lalland Scots / Doric", "Ah can eat gless, it disnae hurt us."},
{0,"3.49","Gothic (4)", "𐌼𐌰𐌲 𐌲𐌻𐌴𐍃 𐌹̈𐍄𐌰𐌽, 𐌽𐌹 𐌼𐌹𐍃 𐍅𐌿 𐌽𐌳𐌰𐌽 𐌱𐍂𐌹𐌲𐌲𐌹𐌸."},
{0,"3.50","Old Norse /(Runes)", "/ ᛖᚴ ᚷᛖᛏ ᛖᛏᛁ ᚧ ᚷᛚᛖᚱ ᛘᚾ ᚦᛖᛋᛋ ᚨᚧ ᚡᛖ ᚱᚧᚨ ᛋᚨᚱ"},
{0,"3.51","Old Norse /(Latin)", "/ Ek get etið gler án þess að verða sár."},
{0,"3.52","Norsk / Norwegian (Nynorsk)", " Eg kan eta glas utan å skada meg."},
{0,"3.53","Norsk / Norwegian (Bokmål)", " Jeg kan spise glass uten å skade meg."},
{0,"3.54","Føroyskt / Faroese", "Eg kann eta glas, skaðaleysur."},
{0,"3.55","Íslenska / Icelandic", "Ég get etið gler án þess að meiða mig."},
{0,"3.56","Svenska / Swedish", "Jag kan äta glas utan att skada mig."},
{0,"3.57","Dansk / Danish", "Jeg kan spise glas, det gør ikke ondt på mig."},
{0,"3.58","Sønderjysk", "Æ ka æe glass uhen at det go mæ naue."},
{0,"3.59","Frysk / Frisian", "Ik kin glês ite, it docht me net sear."},
{0,"3.60","Nederlands / Dutch", "Ik kan glas eten, het doet mĳ geen kwaad."},
{0,"3.61","Kirchröadsj/Bôchesserplat", "Iech ken glaas èèse, mer 't deet miech jing pieng."},
{0,"3.62","Afrikaans", "Ek kan glas eet, maar dit doen my nie skade nie."},
{0,"3.63","Lëtzebuergescht / Luxemburgish", "Ech kan Glas iessen, daat deet mir nët wei."},
{0,"3.64","Deutsch / German", "Ich kann Glas essen, ohne mir zu schaden."},
{0,"3.65","Ruhrdeutsch", "Ich kann Glas verkasematuckeln, ohne dattet mich wat jucken tut."},
{0,"3.66","Langenfelder Platt", "Isch kann Jlaas kimmeln, uuhne datt mich datt weh dääd."},
{0,"3.67","Lausitzer Mundart (Lusatian)", "Ich koann Gloos assn und doas dudd merr ni wii."},
{0,"3.68","Odenwälderisch", "Iech konn glaasch voschbachteln ohne dass es mir ebbs daun doun dud."},
{0,"3.69","Sächsisch / Saxon", "'sch kann Glos essn, ohne dass'sch mer wehtue."},
{0,"3.70","Pfälzisch", "Isch konn Glass fresse ohne dasses mer ebbes ausmache dud."},
{0,"3.71","Schwäbisch / Swabian", "I kå Glas frässa, ond des macht mr nix!"},
{0,"3.72","Deutsch (Voralberg)", "I ka glas eassa, ohne dass mar weh tuat."},
{0,"3.73","Bayrisch / Bavarian", "I koh Glos esa, und es duard ma ned wei."},
{0,"3.74","Allemannisch", "I kaun Gloos essen, es tuat ma ned weh."},
{0,"3.75","Schwyzerdütsch (Zürich)", "Ich chan Glaas ässe, das schadt mir nöd."},
{0,"3.76","Schwyzerdütsch (Luzern)", "Ech cha Glâs ässe, das schadt mer ned."},
{0,"3.77","Hungarian", "Meg tudom enni az üveget, nem lesz tőle bajom."},
{0,"3.78","Suomi / Finnish", "Voin syödä lasia, se ei vahingoita minua."},
{0,"3.79","Sami (Northern)", "Sáhtán borrat lása, dat ii leat bávččas."},
{0,"3.80","Erzian", "Мон ярсан суликадо, ды зыян эйстэнзэ а ули."},
{0,"3.81","Northern Karelian", "Mie voin syvvä lasie ta minla ei ole kipie."},
{0,"3.82","Southern Karelian", "Minä voin syvvä st'oklua dai minule ei ole kibie."},
{0,"3.83","Estonian", "Ma võin klaasi süüa, see ei tee mulle midagi."},
{0,"3.84","Latvian", "Es varu ēst stiklu, tas man nekaitē."},
{0,"3.85","Lithuanian", "Aš galiu valgyti stiklą ir jis manęs nežeidžia"},
{0,"3.86","Czech", "Mohu jíst sklo, neublíží mi."},
{0,"3.87","Slovak", "Môžem jesť sklo. Nezraní ma."},
{0,"3.88","Polska / Polish", "Mogę jeść szkło i mi nie szkodzi."},
{0,"3.89","Slovenian", "Lahko jem steklo, ne da bi mi škodovalo."},
{0,"3.90","Bosnian, Croatian, Montenegrin and Serbian /(Latin)/", "Ja mogu jesti staklo, i to mi ne šteti."},
{0,"3.91","Bosnian, Montenegrin and Serbian /(Cyrillic)/", "Ја могу јести стакло, и то ми не штети."},
{0,"3.92","Macedonian", "Можам да јадам стакло, а не ме штета."},
{0,"3.93","Russian", "Я могу есть стекло, оно мне не вредит."},
{0,"3.94","Belarusian /(Cyrillic)", "Я магу есці шкло, яно мне не шкодзіць."},
{0,"3.95","Belarusian /(Lacinka)", "Ja mahu jeści škło, jano mne ne škodzić."},
{0,"3.96","Ukrainian", "Я можу їсти скло, і воно мені не зашкодить."},
{0,"3.97","Bulgarian", "Мога да ям стъкло, то не ми вреди."},
{0,"3.98","Georgian", "მინას ვჭამ და არა მტკივა."},
{0,"3.99","Armenian", "Կրնամ ապակի ուտել և ինծի անհանգիստ չըներ։"},
{0,"3.100","Albanian", "Unë mund të ha qelq dhe nuk më gjen gjë."},
{0,"3.101","Turkish", "Cam yiyebilirim, bana zararı dokunmaz."},
{0,"3.102","Turkish /(Ottoman)", "جام ييه بلورم بڭا ضررى طوقونمز"},
{0,"3.103","Uzbek / O’zbekcha /(Roman)", "Men shisha yeyishim mumkin, ammo u menga zarar keltirmaydi."},
{0,"3.104","Uzbek / Ўзбекча /(Cyrillic)/", "Мен шиша ейишим мумкин, аммо у менга зарар келтирмайди"},
{0,"3.105","Bangla / Bengali", "আমি কাঁচ খেতে পারি, তাতে আমার কোনো ক্ষতি হয় না।"},
{0,"3.106","Marathi", "मी काच खाऊ शकतो, मला ते दुखत नाही."},
{0,"3.107","Kannada", "ನನಗೆ ಹಾನಿ ಆಗದೆ, ನಾನು ಗಜನ್ನು ತಿನಬಹುದು"},
{0,"3.108","Hindi", "मैं काँच खा सकता हूँ और मुझे उससे कोई चोट नहीं पहुंचती."},
{0,"3.109","Malayalam", "എനിക്ക് ഗ്ലാസ് തിന്നാം. അതെന്നെ വേദനിപ്പിക്കില്ല."},
{0,"3.110","Tamil", "நான் கண்ணாடி சாப்பிடுவேன், அதனால் எனக்கு ஒரு கேடும் வராது."},
{0,"3.111","Telugu", "నేను గాజు తినగలను మరియు అలా చేసినా నాకు ఏమి ఇబ్బంది లేదు"},
{0,"3.112","Sinhalese", "මට වීදුරු කෑමට හැකියි. එයින් මට කිසි හානියක් සිදු නොවේ."},
{0,"3.113","Urdu(3)", "میں کانچ کھا سکتا ہوں اور مجھے تکلیف نہیں ہوتی ۔"},
{0,"3.114","Pashto(3)", "زه شيشه خوړلې شم، هغه ما نه خوږوي"},
{0,"3.115","Farsi / Persian(3)", ".من می توانم بدونِ احساس درد شيشه بخورم"},
{0,"3.116","Arabic(3)", "أنا قادر على أكل الزجاج و هذا لا يؤلمني."},
{0,"3.117","Maltese", "Nista' niekol il-ħġieġ u ma jagħmilli xejn."},
{0,"3.118","Hebrew(3)", "אני יכול לאכול זכוכית וזה לא מזיק לי."},
{0,"3.119","Yiddish(3)", "איך קען עסן גלאָז און עס טוט מיר נישט װײ."},
{0,"3.120","Twi", "Metumi awe tumpan, ɜnyɜ me hwee."},
{0,"3.121","Hausa (/Latin/)", "Inā iya taunar gilāshi kuma in gamā lāfiyā."},
{0,"3.122","Hausa (/Ajami/) (2)", "إِنا إِىَ تَونَر غِلَاشِ كُمَ إِن غَمَا لَافِىَا"},
{0,"3.123","Yoruba(4)", "Mo lè je̩ dígí, kò ní pa mí lára."},
{0,"3.124","Lingala", "Nakokí kolíya biténi bya milungi, ekosála ngáí mabé tɛ́."},
{0,"3.125","(Ki)Swahili", "Naweza kula bilauri na sikunyui."},
{0,"3.126","Malay", "Saya boleh makan kaca dan ia tidak mencederakan saya."},
{0,"3.127","Tagalog", "Kaya kong kumain nang bubog at hindi ako masaktan."},
{0,"3.128","Chamorro", "Siña yo' chumocho krestat, ti ha na'lalamen yo'."},
{0,"3.129","Fijian", "Au rawa ni kana iloilo, ia au sega ni vakacacani kina."},
{0,"3.130","Javanese", "Aku isa mangan beling tanpa lara."},
{0,"3.131","Burmese (Unicode 4.0)", "က္ယ္ဝန္‌တော္‌၊က္ယ္ဝန္‌မ မ္ယက္‌စားနုိင္‌သည္‌။ ၎က္ရောင္‌့ ထိခုိက္‌မ္ဟု မရ္ဟိပာ။"},
{0,"3.132","Burmese (Unicode 5.0)", "ကျွန်တော် ကျွန်မ မှန်စားနိုင်တယ်။ ၎င်းကြောင့် ထိခိုက်မှုမရှိပါ။"},
{0,"3.133","Vietnamese (quốc ngữ)", "Tôi có thể ăn thủy tinh mà không hại gì."},
{0,"3.134","Vietnamese (nôm) (4)", "些 𣎏 世 咹 水 晶 𦓡 空 𣎏 害 咦"},
{0,"3.135","Khmer", "ខ្ញុំអាចញុំកញ្ចក់បាន ដោយគ្មានបញ្ហារ"},
{0,"3.136","Lao", "ຂອ້ຍກິນແກ້ວໄດ້ໂດຍທີ່ມັນບໍ່ໄດ້ເຮັດໃຫ້ຂອ້ຍເຈັບ."},
{0,"3.137","Thai", "ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ"},
{0,"3.138","Mongolian /(Cyrillic)", "Би шил идэй чадна, надад хортой биш"},
{0,"3.139","Mongolian /(Classic)/ (5)", "ᠪᠢ ᠰᠢᠯᠢ ᠢᠳᠡᠶᠦ ᠴᠢᠳᠠᠨᠠ ᠂ ᠨᠠᠳᠤᠷ ᠬᠣᠤᠷᠠᠳᠠᠢ ᠪᠢᠰᠢ"},
{0,"3.140","Nepali", "﻿म काँच खान सक्छू र मलाई केहि नी हुन्‍न् ।"},
{0,"3.141","Tibetan", "ཤེལ་སྒོ་ཟ་ནས་ང་ན་གི་མ་རེད།"},
{0,"3.142","Chinese", "我能吞下玻璃而不伤身体。"},
{0,"3.143","Chinese (Traditional)", "我能吞下玻璃而不傷身體。"},
{0,"3.144","Taiwanese(6)", "Góa ē-tàng chia̍h po-lê, mā bē tio̍h-siong."},
{0,"3.145","Japanese", "私はガラスを食べられます。それは私を傷つけません。"},
{0,"3.146","Korean", "나는 유리를 먹을 수 있어요. 그래도 아프지 않아요"},
{0,"3.147","Bislama", "Mi save kakae glas, hemi no save katem mi."},
{0,"3.148","Hawaiian", "Hiki iaʻu ke ʻai i ke aniani; ʻaʻole nō lā au e ʻeha."},
{0,"3.149","Marquesan", "E koʻana e kai i te karahi, mea ʻā, ʻaʻe hauhau."},
{0,"3.150","Inuktitut (10)", "ᐊᓕᒍᖅ ᓂᕆᔭᕌᖓᒃᑯ ᓱᕋᙱᑦᑐᓐᓇᖅᑐᖓ"},
{0,"3.151","Chinook Jargon", "Naika məkmək kakshət labutay, pi weyk ukuk munk-sik nay."},
{0,"3.152","Navajo", "Tsésǫʼ yishą́ągo bííníshghah dóó doo shił neezgai da."},
{0,"3.153","Lojban", "mi kakne le nu citka le blaci .iku'i le se go'i na xrani mi"},
{0,"3.154","Nórdicg", "Ljœr ye caudran créneþ ý jor cẃran."},
NULLTEST
};

static const struct Test utf8phrases2[] = {
{0,"4.1","Euro Symbol", "€."},
{0,"4.2","Greek", "Μπορώ να φάω σπασμένα γυαλιά χωρίς να πάθω τίποτα."},
{0,"4.3","Íslenska / Icelandic", "Ég get etið gler án þess að meiða mig."},
{0,"4.4","Polish", "Mogę jeść szkło, i mi nie szkodzi."},
{0,"4.5","Romanian", "Pot să mănânc sticlă și ea nu mă rănește."},
{0,"4.6","Ukrainian", "Я можу їсти шкло, й воно мені не пошкодить."},
{0,"4.7","Armenian", "Կրնամ ապակի ուտել և ինծի անհանգիստ չըներ։"},
{0,"4.8","Georgian", "მინას ვჭამ და არა მტკივა."},
{0,"4.9","Hindi", "मैं काँच खा सकता हूँ, मुझे उस से कोई पीडा नहीं होती."},
{0,"4.10", "Hebrew", "אני יכול לאכול זכוכית וזה לא מזיק לי."},
{0,"4.11","Yiddish", "איך קען עסן גלאָז און עס טוט מיר נישט װײ."},
{0,"4.12","Arabic", "أنا قادر على أكل الزجاج و هذا لا يؤلمني."},
{0,"4.13","Japanese", "私はガラスを食べられます。それは私を傷つけません。"},
{0,"4.14","Thai", "ฉันกินกระจกได้ แต่มันไม่ทำให้ฉันเจ็บ "},
NULLTEST
};

static char*
trim(const char* s)
{
    size_t i;
    size_t l = strlen(s);
    char* t = strdup(s);
    for(i=l; i--> 0;) {
        if(t[i] != ' ') break;
    }
    t[i+1] = '\0';
    return t;
}

static int
test(const struct Test* tests, const char* title)
{
    int status = NC_NOERR;
    int failures = 0;
    const struct Test* p;

    fprintf(stderr,"Testing %s...\n",title);
    for(p=tests;p->id;p++) {
	unsigned char* normal;
	char* id;
        char* description;
        const char* pf;
        id = trim(p->id);
        description = trim(p->description);
	/* 1. validate the string */
        status = nc_utf8_validate((const unsigned char*)p->data);
        if(status != NC_NOERR) {pf = "Fail"; failures++; goto fail;}
	/* 2. normalize the string */
        status = nc_utf8_normalize((const unsigned char*)p->data,&normal);
        if(status != NC_NOERR) {pf = "Fail"; failures++; goto fail;}
	/* 3. re-validate the normalized string */
        status = nc_utf8_validate((const unsigned char*)normal);
        if(status != NC_NOERR) {pf = "Fail"; failures++; goto fail;}
	/* 3. compare input with output */
	{
	    size_t dlen = strlen((const char*)p->data);
	    size_t nlen = strlen((const char*)normal);
	    if(dlen != nlen)
		fprintf(stderr,"\t%s: length mismatch: in=%zu norm=%zu\n",p->id,dlen,nlen);
	    size_t mlen = (dlen < nlen ? dlen : nlen);
	    for(size_t i=0;i<mlen;i++) {
		unsigned char cd = (unsigned char)p->data[i];
		unsigned char cn = normal[i];
		if(cd != cn) {
		    fprintf(stderr,"\t%s: [%zu] data=|%02x| normal=|%02x|\n",p->id,i,cd,cn);
		    break;
		}
	    }
	}
	pf = "Pass";
	free(normal);
fail:
        fprintf(stderr,"%s: %s %s\n",pf,id,description);
        fflush(stderr);
	free(id);
	free(description);
    }
    return failures;
}

int
main(int argc, char** argv)
{
    int failures = 0;

    printf("\n Testing UTF-8 sequences.\n");
    failures += test(utf8currency,"Currencies");
    failures += test(utf8poems,"Poetry");
    failures += test(utf8phrases1,"Phrases Set 1");
    failures += test(utf8phrases2,"Phrases Set 2");
    fprintf(stderr,"No. of failures = %d\n",failures);
    exit(failures == 0 ? 0 : 1);
}
