/*

	GeoIP Include by Whitetiger
	-> SQLite GeoIP Include
	
	Credits: Whitetiger, RaekwonDaChef, Y_Less, Andreas Gohr.
	
	MaxMind, GeoIP and related marks are registered trademarks of MaxMind, Inc.
	
	---------------------------------------------------------------------------------------
	
*/

#if defined _geolocation_included
  #endinput
#endif


#define _geolocation_included

#include <a_samp>


/*
	---------------------------------------------------------------------------------------
	Variables
	---------------------------------------------------------------------------------------
*/


#if !defined GeoIP_MainFile
	#define GeoIP_MainFile      "geoip.db"
#endif

#if !defined GeoIP_CityFile
	#define GeoIP_CityFile      "geoip_city.db"
#endif

new DB:geoip_db;
new DB:geoip_city;

new stock g_iBackwardsCompat = -1;


new DBResult:_result;

/*
	---------------------------------------------------------------------------------------
	NATIVES
	---------------------------------------------------------------------------------------
*/

/*
	native GetPlayerCountry(playerid, string[], const len = sizeof(string));
	native GetPlayerISP(playerid, string[], const len = sizeof(string));
	native GetPlayerCity(playerid, string[], const len = sizeof(string));
	native GetPlayerGMT(playerid);
	native GetPlayerProxy(playerid);
*/

stock GetPlayerCountry(playerid, string[], const len = sizeof(string)) {
	new ip[24];
	GetPlayerIp(playerid, ip, sizeof(ip));

	GetIPCountry(ip, string, len);

	return true;
}

stock GetPlayerISP(playerid, string[], const len = sizeof(string)) {
	new ip[24];
	GetPlayerIp(playerid, ip, sizeof(ip));

	GetIPISP(ip, string, len);
	new placeholder[1];
    sscanf_(string, "ss", placeholder, string);

    return true;
}

stock GetPlayerCity(playerid, string[], const len = sizeof(string)) {
	new ip[24];
	GetPlayerIp(playerid, ip, sizeof(ip));

	GetIPCity(ip, string, len);

	return true;
}

stock GetPlayerGMT(playerid) {
	new ip[24];
	GetPlayerIp(playerid, ip, sizeof(ip));

	return GetIPGMT(ip);
}

stock GetPlayerProxy(playerid) {
	new ip[24];
	GetPlayerIp(playerid, ip, sizeof(ip));
	
	return GetIPProxy(ip);
}

/*

	---------------------------------------------------------------------------------------
	INTERNAL FUNCTIONS
	---------------------------------------------------------------------------------------

	- stock GetIPCountry(ip[], dest[], len = sizeof(dest))
	- stock GetIPISP(ip[], dest[], len = sizeof(dest))
	- stock GetIPCity(ip[], dest[], len = sizeof(dest))
	- stock GetIPGMT(ip[])
	- stock GetIPProxy(ip[])
	- stock ip2long(ip[])

*/

stock GetIPCountry(ip[], dest[], const len = sizeof(dest)) {
	new tmp[90];
	tmp = ip2long(ip);
	
	new string[500];
	
	geoip_db = db_open(GeoIP_MainFile);
	
	if(g_iBackwardsCompat == -1) {
	    _result = db_query(geoip_db, "SELECT name FROM sqlite_master WHERE type='table' AND name='ip_country'");
	    if(db_num_rows(_result) >= 1)
        {
            g_iBackwardsCompat = 1;
        }
        else
        {
            g_iBackwardsCompat = 0;
		}
		db_free_result(_result);
	}
	
	if(g_iBackwardsCompat == 1) {
	    format(string, sizeof(string), "SELECT cn FROM ip_country WHERE idx >= (%s-(%s %% 65536)) AND ip_to >= %s AND  ip_from < %s LIMIT 1", tmp, tmp, tmp, tmp);
	} else {
		format(string, sizeof(string), "SELECT loc.*\n FROM loc_country loc,\n blocks_country blk\n WHERE blk.idx >= (%s-(%s %% 65536))\n AND blk.startIpNum < %s\n AND blk.endIpNum > %s\n AND loc.locId = blk.locId LIMIT 1;", tmp, tmp, tmp, tmp);
	}

    _result = db_query(geoip_db, string);
    if(db_num_rows(_result) >= 1)
    {
  		db_get_field_assoc(_result,"cn",dest,len);
    }
    db_free_result(_result);
	db_close(geoip_db);
	
	if(!strlen(dest)) format(dest, len, "Unknown");
	
	return true;
}

stock GetIPISP(ip[], dest[], const len = sizeof(dest)) {

	new tmp[90];
	tmp = ip2long(ip);
	
	new string[500];
	format(string, sizeof(string), "SELECT internet_service_provider FROM geo_isp WHERE idx >= (%s-(%s %% 65536)) AND ip_to >= %s AND  ip_from < %s LIMIT 1", tmp, tmp, tmp, tmp);

	geoip_db = db_open(GeoIP_MainFile);
    _result = db_query(geoip_db, string);
    if(db_num_rows(_result) >= 1)
    {
        db_get_field_assoc(_result,"internet_service_provider",dest,len);
    }
    db_free_result(_result);
	db_close(geoip_db);
	
	if(!strlen(dest)) format(dest, len, "Unknown");

	return true;
}

stock GetIPCity(ip[], dest[], const len = sizeof(dest)) {
	new tmp[90];
	tmp = ip2long(ip);
	
	new string[500];
	format(string, sizeof(string), "SELECT loc.*\n FROM geolocation loc,\n geoblocks blk\n WHERE blk.idx = (%s-(%s %% 65536))\n AND blk.startIpNum < %s\n AND blk.endIpNum > %s\n AND loc.locId = blk.locId LIMIT 1;", tmp, tmp, tmp, tmp);

	geoip_city = db_open(GeoIP_CityFile);
    _result = db_query(geoip_city, string);
    if(db_num_rows(_result) >= 1)
    {
        db_get_field_assoc(_result,"city",dest,len);
    }
    db_free_result(_result);
	db_close(geoip_city);
	
	if(!strlen(dest)) format(dest, len, "Unknown");
	
	return true;
}

stock GetIPGMT(ip[]) {
	new tmp[90];
	tmp = ip2long(ip);
	
	new dest[50];
	
	new string[500];
	format(string, sizeof(string), "SELECT blk.*, loc.longitude\n FROM geolocation loc,\n geoblocks blk\n WHERE blk.idx = (%s-(%s %% 65536))\n AND blk.startIpNum < %s\n AND blk.endIpNum > %s\n AND loc.locId = blk.locId LIMIT 1;", tmp, tmp, tmp, tmp);

	geoip_city = db_open(GeoIP_CityFile);
    _result = db_query(geoip_city, string);
    if(db_num_rows(_result) >= 1)
    {
        db_get_field_assoc(_result, "longitude", dest, sizeof(dest));
    }
    db_free_result(_result);
	db_close(geoip_city);
	
	if(!strlen(dest)) format(dest, sizeof(dest), "Unknown");
	
	return floatround( strval( dest ) / 15 );
}

stock GetIPProxy(ip[]) {

    /*if(g_iBackwardsCompat == -1) {
	    _result = db_query(geoip_db, "SELECT name FROM sqlite_master WHERE type='table' AND name='ip_country'");
	    if(db_num_rows(_result) >= 1)
        {
            g_iBackwardsCompat = 1;
        }
        else
        {
            g_iBackwardsCompat = 0;
		}
	}
	
	if(g_iBackwardsCompat == 1) {
	    printf("Error: GetIPProxy cannot work on old GeoIP Database files, please update your database files!");
	    printf("http://forum.sa-mp.com/showthread.php?t=296171");
	    return false;
	}
	if(strlen(ip) > 0) {
	
		new tmp[90];
		tmp = ip2long(ip);

		new dest[50];

		new string[500];
		format(string, sizeof(string), "SELECT blk.*\n FROM geolocation loc,\n geoblocks blk\n WHERE blk.idx = (%s-(%s %% 65536))\n AND blk.startIpNum < %s\n AND blk.endIpNum > %s\n AND loc.locId = blk.locId LIMIT 1;", tmp, tmp, tmp, tmp);

		geoip_city = db_open(GeoIP_CityFile);
		_result = db_query(geoip_city, string);
		if(db_num_rows(_result) >= 1)
		{
			db_get_field_assoc(_result, "is_anonymous_proxy", dest, sizeof(dest));
		}
		db_free_result(_result);
		db_close(geoip_city);
		
		return strval(dest);
	}*/
	
	#pragma unused ip
	return false;
}

/*forward OnProxyRecieved(index, response_code, data[]);
public OnProxyRecieved(index, response_code, data[]) {

	if(IsPlayerConnected(index)) {
	
		new ip[MAX_PLAYER_NAME];
		GetPlayerIp(index, ip, sizeof(ip));
		
		if(strlen(IPInfo[index]) > 0 && strlen(ip) > 0 && !strcmp(IPInfo[index], ip, true)) {
			CallLocalFunction("OnProxyResponse", "ii",  index, strval(data));
		}
	}
	IPInfo[index] = "";
}*/

stock ip2long(ip[]) {
	new ips[4];
	sscanf_(ip, "p.dddd", ips[0], ips[1], ips[2], ips[3]);
	new tmp[90];
	format(tmp, sizeof(tmp), "((16777216 * %d) + (65536 * %d) + (256 * %d) + %d)", ips[0], ips[1], ips[2], ips[3]);
	// we use a string here so it will not pass the 32-bit integer limits, the math is done later in the sql query
	return tmp;
}

/*

	---------------------------------------------------------------------------------------
	SECONDARY FUNCTIONS
	---------------------------------------------------------------------------------------
	
	- stock sscanf_(string[], format[], {Float,_}:...)

*/

stock sscanf_(string[], format[], {Float,_}:...) // by Y_Less
{
	#if defined isnull
		if (isnull(string))
	#else
		if (string[0] == 0 || (string[0] == 1 && string[1] == 0))
	#endif
		{
			return format[0];
		}
	#pragma tabsize 4
	new
		formatPos = 0,
		stringPos = 0,
		paramPos = 2,
		paramCount = numargs(),
		delim = ' ';
	while (string[stringPos] && string[stringPos] <= ' ')
	{
		stringPos++;
	}
	while (paramPos < paramCount && string[stringPos])
	{
		switch (format[formatPos++])
		{
			case '\0':
			{
				return 0;
			}
			case 'i', 'd':
			{
				new
					neg = 1,
					num = 0,
					ch = string[stringPos];
				if (ch == '-')
				{
					neg = -1;
					ch = string[++stringPos];
				}
				do
				{
					stringPos++;
					if ('0' <= ch <= '9')
					{
						num = (num * 10) + (ch - '0');
					}
					else
					{
						return -1;
					}
				}
				while ((ch = string[stringPos]) > ' ' && ch != delim);
				setarg(paramPos, 0, num * neg);
			}
			case 'h', 'x':
			{
				new
					num = 0,
					ch = string[stringPos];
				do
				{
					stringPos++;
					switch (ch)
					{
						case 'x', 'X':
						{
							num = 0;
							continue;
						}
						case '0' .. '9':
						{
							num = (num << 4) | (ch - '0');
						}
						case 'a' .. 'f':
						{
							num = (num << 4) | (ch - ('a' - 10));
						}
						case 'A' .. 'F':
						{
							num = (num << 4) | (ch - ('A' - 10));
						}
						default:
						{
							return -1;
						}
					}
				}
				while ((ch = string[stringPos]) > ' ' && ch != delim);
				setarg(paramPos, 0, num);
			}
			case 'c':
			{
				setarg(paramPos, 0, string[stringPos++]);
			}
			case 'f':
			{

				new changestr[16], changepos = 0, strpos = stringPos;
				while(changepos < 16 && string[strpos] && string[strpos] != delim)
				{
					changestr[changepos++] = string[strpos++];
   				}
				changestr[changepos] = '\0';
				setarg(paramPos,0,_:floatstr(changestr));
			}
			case 'p':
			{
				delim = format[formatPos++];
				continue;
			}
			case '\'':
			{
				new
					end = formatPos - 1,
					ch;
				while ((ch = format[++end]) && ch != '\'') {}
				if (!ch)
				{
					return -1;
				}
				format[end] = '\0';
				if ((ch = strfind(string, format[formatPos], false, stringPos)) == -1)
				{
					if (format[end + 1])
					{
						return -1;
					}
					return 0;
				}
				format[end] = '\'';
				stringPos = ch + (end - formatPos);
				formatPos = end + 1;
			}
			case 'u':
			{
				new
					end = stringPos - 1,
					id = 0,
					bool:num = true,
					ch;
				while ((ch = string[++end]) && ch != delim)
				{
					if (num)
					{
						if ('0' <= ch <= '9')
						{
							id = (id * 10) + (ch - '0');
						}
						else
						{
							num = false;
						}
					}
				}
				if (num && IsPlayerConnected(id))
				{
					setarg(paramPos, 0, id);
				}
				else
				{
					#if !defined foreach
						#define foreach(%1,%2) for (new %2 = 0; %2 < MAX_PLAYERS; %2++) if (IsPlayerConnected(%2))
						#define __SSCANF_FOREACH__
					#endif
					string[end] = '\0';
					num = false;
					new
						name[MAX_PLAYER_NAME];
					id = end - stringPos;
					foreach (Player, playerid)
					{
						GetPlayerName(playerid, name, sizeof (name));
						if (!strcmp(name, string[stringPos], true, id))
						{
							setarg(paramPos, 0, playerid);
							num = true;
							break;
						}
					}
					if (!num)
					{
						setarg(paramPos, 0, INVALID_PLAYER_ID);
					}
					string[end] = ch;
					#if defined __SSCANF_FOREACH__
						#undef foreach
						#undef __SSCANF_FOREACH__
					#endif
				}
				stringPos = end;
			}
			case 's', 'z':
			{
				new
					i = 0,
					ch;
				if (format[formatPos])
				{
					while ((ch = string[stringPos++]) && ch != delim)
					{
						setarg(paramPos, i++, ch);
					}
					if (!i)
					{
						return -1;
					}
				}
				else
				{
					while ((ch = string[stringPos++]))
					{
						setarg(paramPos, i++, ch);
					}
				}
				stringPos--;
				setarg(paramPos, i, '\0');
			}
			default:
			{
				continue;
			}
		}
		while (string[stringPos] && string[stringPos] != delim && string[stringPos] > ' ')
		{
			stringPos++;
		}
		while (string[stringPos] && (string[stringPos] == delim || string[stringPos] <= ' '))
		{
			stringPos++;
		}
		paramPos++;
	}
	do
	{
		if ((delim = format[formatPos++]) > ' ')
		{
			if (delim == '\'')
			{
				while ((delim = format[formatPos++]) && delim != '\'') {}
			}
			else if (delim != 'z')
			{
				return delim;
			}
		}
	}
	while (delim > ' ');
	return 0;
}
