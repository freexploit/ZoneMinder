//
// ZoneMinder Configuration Implementation, $Date$, $Revision$
// Copyright (C) 2003  Philip Coombes
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 

#include <string.h>
#include <stdlib.h>

#include "zm.h"
#include "zm_db.h"

ConfigItem::ConfigItem( const char *p_name, const char *p_value, const char *const p_type )
{
	name = new char[strlen(p_name)+1];
	strcpy( name, p_name );
	value = new char[strlen(p_value)+1];
	strcpy( value, p_value );
	type = new char[strlen(p_type)+1];
	strcpy( type, p_type );

	//Info(( "Created new config item %s = %s (%s)\n", name, value, type ));

	accessed = false;
}

ConfigItem::~ConfigItem()
{
	delete[] name;
	delete[] value;
	delete[] type;
}

void ConfigItem::ConvertValue() const
{
	if ( !strcmp( type, "boolean" ) )
	{
		cfg_type = CFG_BOOLEAN;
		cfg_value.boolean_value = strtol( value, 0, 2 );
	}
	else if ( !strcmp( type, "integer" ) )
	{
		cfg_type = CFG_INTEGER;
		cfg_value.integer_value = strtol( value, 0, 10 );
	}
	else if ( !strcmp( type, "hexadecimal" ) )
	{
		cfg_type = CFG_INTEGER;
		cfg_value.integer_value = strtol( value, 0, 16 );
	}
	else if ( !strcmp( type, "decimal" ) )
	{
		cfg_type = CFG_DECIMAL;
		cfg_value.decimal_value = atof( value );
	}
	else
	{
		cfg_type = CFG_STRING;
		cfg_value.string_value = value;
	}
	accessed = true;
}

bool ConfigItem::BooleanValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_BOOLEAN )
		Warning(( "Attempt to fetch boolean value for %s, actual type is %s", name, type ));

	return( cfg_value.boolean_value );
}

int ConfigItem::IntegerValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_INTEGER )
		Warning(( "Attempt to fetch integer value for %s, actual type is %s", name, type ));

	return( cfg_value.integer_value );
}

double ConfigItem::DecimalValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_DECIMAL )
		Warning(( "Attempt to fetch decimal value for %s, actual type is %s", name, type ));

	return( cfg_value.decimal_value );
}

const char *ConfigItem::StringValue() const
{
	if ( !accessed )
		ConvertValue();

	if ( cfg_type != CFG_STRING )
		Warning(( "Attempt to fetch string value for %s, actual type is %s", name, type ));

	return( cfg_value.string_value );
}

Config::Config()
{
	n_items = 0;
	items = 0;
}

Config::~Config()
{
	for ( int i = 0; i < n_items; i++ )
	{
		delete items[i];
	}
	delete[] items;
	n_items = 0;
}

void Config::Load()
{
	static char sql[BUFSIZ];

	strcpy( sql, "select Name, Value, Type from Config order by Id" );
	if ( mysql_query( &dbconn, sql ) )
	{
		Error(( "Can't run query: %s", mysql_error( &dbconn ) ));
		exit( mysql_errno( &dbconn ) );
	}

	MYSQL_RES *result = mysql_store_result( &dbconn );
	if ( !result )
	{
		Error(( "Can't use query result: %s", mysql_error( &dbconn ) ));
		exit( mysql_errno( &dbconn ) );
	}
	n_items = mysql_num_rows( result );
	items = new ConfigItem *[n_items];
	for( int i = 0; MYSQL_ROW dbrow = mysql_fetch_row( result ); i++ )
	{
		items[i] = new ConfigItem( dbrow[0], dbrow[1], dbrow[2] );
	}
}

const ConfigItem &Config::Item( int id )
{
	if ( !n_items )
	{
		Load();
	}

	if ( id < 0 || id > ZM_MAX_CFG_ID )
	{
		Error(( "Attempt to access invalid config, id = %d", id ));
		exit( -1 );
	}

	ConfigItem *item = items[id];
	
	if ( !item )
	{
		Error(( "Can't find config item %d", id ));
		exit( -1 );
	}
		
	return( *item );
}

Config config;