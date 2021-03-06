/*
* https://github.com/condolent/Better-Warden
*/

#if defined _GANGS_included
 #endinput
#endif
#define _GANGS_included

Database gH_Db;

/**
* Initiates the database connection
*
* @error				Cannot create DB connection
*/
stock void SQL_InitDB() {
	char sError[255];
	if(gH_Db == INVALID_HANDLE) {
		gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
		
		if(gH_Db == INVALID_HANDLE) {
			PrintToServer("### Could not connect to the database! ##");
			PrintToServer("%s", sError);
		} else {
			SQL_CreateBWTable();
		}
	}
}

/**
* Creates the table if it doesn't exist
*
* @error				Database connection issues
*/
stock void SQL_CreateBWTable() {
	char sQuery[255];
	char sQuery2[255];
	Format(sQuery, sizeof(sQuery), "CREATE TABLE IF NOT EXISTS betterwarden_players (ID int NOT NULL AUTO_INCREMENT, STEAMID varchar(255), GangID int, PRIMARY KEY(ID));");
	Format(sQuery2, sizeof(sQuery2), "CREATE TABLE IF NOT EXISTS betterwarden_gangs (ID int NOT NULL AUTO_INCREMENT, Name varchar(255), Owner varchar(255), PRIMARY KEY(ID));");
	
	if(!SQL_FastQuery(gH_Db, sQuery)) {
		char err[255];
		SQL_GetError(gH_Db, err, sizeof(err));
		PrintToServer("Error: %s", err);
	}
	
	if(!SQL_FastQuery(gH_Db, sQuery2)) {
		char err[255];
		SQL_GetError(gH_Db, err, sizeof(err));
		PrintToServer("Error: %s", err);
	}
	
	CloseHandle(gH_Db);
}

/**
* Checks if a user is registered or not in the DB
*
* @param client			The client to look up
* @return				true if exists
*/
stock bool SQL_UserExists(int client) {
	char sQuery[255];
	char sSteamID[64];
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	GetClientAuthId(client, AuthId_Steam2, sSteamID, sizeof(sSteamID));
	
	Format(sQuery, sizeof(sQuery), "SELECT * FROM betterwarden_players WHERE STEAMID LIKE '%s'", sSteamID);
	
	DBResultSet SQL = SQL_Query(gH_Db, sQuery);
	
	if(gH_Db != INVALID_HANDLE) {
		if(SQL != INVALID_HANDLE) {
			if(SQL_FetchRow(SQL)) {
				CloseHandle(SQL);
				CloseHandle(gH_Db);
				return true;
			}
			CloseHandle(gH_Db);
			return false;
		} else {
			PrintToServer("#################");
			PrintToServer("INVALID_HANDLE in SQL_UserExists()");
		}
	} else {
		PrintToServer("#################");
		PrintToServer("Database = INVALID_HANDLE in SQL_UserExists(): %s", sError);
	}
	
	CloseHandle(gH_Db);
	CloseHandle(SQL);
	return false;
}

/**
* Adds a user to the database
*
* @param client			The client to add
* @error				Invalid client or database connection issues
*/
stock void SQL_AddUserToBWTable(int client) {
	char sQuery[255];
	char sSteamID[64];
	GetClientAuthId(client, AuthId_Steam2, sSteamID, sizeof(sSteamID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	Format(sQuery, sizeof(sQuery), "INSERT INTO betterwarden_players (STEAMID) VALUES ('%s');", sSteamID);
	
	if(!SQL_FastQuery(gH_Db, sQuery)) {
		PrintToServer("Database Error: %s", sError);
	}
	
	CloseHandle(gH_Db);
}

/**
* Checks to see if the given gang exists already.
*
* @param name			Gang-name to look up
* @return				true if exists
*/
stock bool SQL_GangExists(char[] name) {
	char sQuery[255];
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	Format(sQuery, sizeof(sQuery), "SELECT * FROM betterwarden_gangs WHERE Name LIKE '%s'", name);
	
	if(gH_Db != INVALID_HANDLE) {
		DBResultSet SQL = SQL_Query(gH_Db, sQuery);
	
		if(SQL != INVALID_HANDLE) {
			if(SQL_FetchRow(SQL)) {
				if(!SQL_IsFieldNull(SQL, 0)) { // Gang exist
					CloseHandle(SQL);
					CloseHandle(gH_Db);
					return true;
				}
				
				// Gang does not exist
				CloseHandle(SQL);
				CloseHandle(gH_Db);
				return false;
			}
			// Gang does not exist
			CloseHandle(SQL);
			CloseHandle(gH_Db);
			return false;
		}
	} else {
		PrintToServer("Database error: %s", sError);
	}
	
	CloseHandle(gH_Db);
	return false;
}

/**
* Creates a gang and registers it to the database
*
* @param client			Client that'll be set as owner
* @param name			Name of the gang
*/
stock void SQL_CreateGang(int client, char[] name) {
	char sQuery[255];
	char sID[64];
	GetClientAuthId(client, AuthId_Steam2, sID, sizeof(sID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	if(gH_Db != INVALID_HANDLE) {
		Format(sQuery, sizeof(sQuery), "INSERT INTO betterwarden_gangs (Name, Owner) VALUES ('%s', '%s');", name, sID);
		
		if(!SQL_FastQuery(gH_Db, sQuery)) {
			PrintToServer("Database Error: %s", sError);
		}
		
		// Add the user to that gang
		int iGang = SQL_GetGangId(name);
		SQL_AddToGang(client, iGang);
		
		CloseHandle(gH_Db);
	}
}

/**
* Checks if a user owns a gang or not
*
* @param client			Client entity index
* @return				true if yes
*/
stock bool SQL_OwnsGang(int client) {
	char sQuery[255];
	char sID[64];
	GetClientAuthId(client, AuthId_Steam2, sID, sizeof(sID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	Format(sQuery, sizeof(sQuery), "SELECT * FROM betterwarden_gangs WHERE Owner LIKE '%s'", sID);
	
	DBResultSet SQL = SQL_Query(gH_Db, sQuery);
	
	if(gH_Db != INVALID_HANDLE) {
		if(SQL != null) {
			if(SQL_FetchRow(SQL)) {
				CloseHandle(SQL);
				CloseHandle(gH_Db);
				return true;
			}
		}
	} else {
		PrintToServer("Database error: %s", sError);
	}
	
	CloseHandle(SQL);
	return false;
}

/**
* Retrieves the gang name of the specified client
* 
* @param client			Client entity index
* @param buffer			Destination string to store the name
* @param maxlen			Max length of the destination buffer
* @error				Not in a gang or invalid client
*/
stock void SQL_GetGang(int client, char[] buffer, int maxlen) {
	int GangID;
	char sQuery[255];
	char sID[64];
	GetClientAuthId(client, AuthId_Steam2, sID, sizeof(sID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	if(gH_Db != INVALID_HANDLE) {
		// Stage one
		Format(sQuery, sizeof(sQuery), "SELECT GangID FROM betterwarden_players WHERE STEAMID LIKE '%s'", sID);
		
		DBResultSet SQL = SQL_Query(gH_Db, sQuery);
		if(SQL != null) {
			if(SQL_FetchRow(SQL)) {
				GangID = SQL_FetchInt(SQL, 0);
				CloseHandle(SQL);
			}
		}
		
		// Stage two
		Format(sQuery, sizeof(sQuery), "SELECT Name FROM betterwarden_gangs WHERE ID LIKE %d", GangID);
		DBResultSet SQL2 = SQL_Query(gH_Db, sQuery);
		if(SQL2 != null) {
			if(SQL_FetchRow(SQL2)) {
				char result[255];
				SQL_FetchString(SQL2, 0, result, sizeof(result));
				CloseHandle(SQL2);
				
				Format(buffer, maxlen, "%s", result);
			}
		}
	} else {
		PrintToServer("Database error: %s", sError);
	}
	
	CloseHandle(gH_Db);
}

/**
* Checks if a client is in a gang
*
* @param client			Client entity index
* @return				true if yes
*/
stock bool SQL_IsInGang(int client) {
	char sQuery[255];
	char sID[64];
	GetClientAuthId(client, AuthId_Steam2, sID, sizeof(sID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	if(gH_Db != INVALID_HANDLE) {
		Format(sQuery, sizeof(sQuery), "SELECT GangID FROM betterwarden_players WHERE STEAMID LIKE '%s'", sID);
		
		DBResultSet SQL = SQL_Query(gH_Db, sQuery);
		if(SQL != null) {
			if(SQL_FetchRow(SQL)) {
				if(SQL_IsFieldNull(SQL, 0)) {
					CloseHandle(SQL);
					CloseHandle(gH_Db);
					return false;
				}
				CloseHandle(SQL);
				CloseHandle(gH_Db);
				return true;
			}
			
			CloseHandle(SQL);
			CloseHandle(gH_Db);
			return false;
		}
	} else {
		PrintToServer("Database error: %s", sError);
	}
	
	return false;
}

/**
* Returns the gang ID from a gang name
*
* @param GangName		Gang name (Non-Case specific)
* @return				Gang ID if successful, -1 if not
*/
stock int SQL_GetGangId(char[] GangName) {
	int gangID;
	char sQuery[255];
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	if(gH_Db != INVALID_HANDLE) {
		Format(sQuery, sizeof(sQuery), "SELECT ID FROM betterwarden_gangs WHERE Name LIKE '%s'", GangName);
		
		DBResultSet SQL = SQL_Query(gH_Db, sQuery);
		if(SQL != null) {
			if(SQL_FetchRow(SQL)) {
				gangID = SQL_FetchInt(SQL, 0);
				CloseHandle(SQL);
				CloseHandle(gH_Db);
				
				return gangID;
			}
			
			CloseHandle(gH_Db);
		}
	} else {
		PrintToServer("Database error: %s", sError);
	}
	
	return -1;
}

/**
* Returns gang ID from the owner of a gang
*
* @param client			Client entity index of the owner
* @return				Gang ID
* @error				Invalid client ID, or client owns no gang (-1)
*/
stock int SQL_GetGangIdFromOwner(int client) {
	char sQuery[255];
	char SteamID[64];
	int gangID;
	GetClientAuthId(client, AuthId_Steam2, SteamID, sizeof(SteamID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	if(gH_Db != INVALID_HANDLE) {
		Format(sQuery, sizeof(sQuery), "SELECT ID FROM betterwarden_gangs WHERE Owner LIKE '%s'", SteamID);
		
		DBResultSet SQL = SQL_Query(gH_Db, sQuery);
		if(SQL != null) {
			if(SQL_FetchRow(SQL)) {
				gangID = SQL_FetchInt(SQL, 0);
				CloseHandle(SQL);
				CloseHandle(gH_Db);
				return gangID;
			}
			
			CloseHandle(gH_Db);
		}
	} else {
		PrintToServer("Database error: %s", sError);
	}
	
	return -1;
}

/**
* Deletes a gang from the database
*
* @param GangID			Gang ID
* @return				true if successful
* @error				Invalid database handle
*/
stock bool SQL_DeleteGang(int GangID) {
	char sQuery[255];
	int client = SQL_GetGangOwner(GangID);
	
	Format(sQuery, sizeof(sQuery), "DELETE FROM betterwarden_gangs WHERE ID = %d", GangID);
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	if(!SQL_FastQuery(gH_Db, sQuery)) {
		PrintToServer("Database Error: %s", sError);
		CloseHandle(gH_Db);
		
		return false;
	}
	
	CloseHandle(gH_Db);
	
	SQL_KickFromGang(client);
	return true;
}

/**
* Kicks a player from a gang
*
* @param client			Kicked client ID
* @return				true if successful
* @error				Invalid client/gang ID, or database connection
*/
stock bool SQL_KickFromGang(int client) {
	char sQuery[255];
	char sID[64];
	GetClientAuthId(client, AuthId_Steam2, sID, sizeof(sID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	Format(sQuery, sizeof(sQuery), "UPDATE betterwarden_players SET GangID = NULL WHERE SteamID LIKE '%s'", sID);
	
	if(!SQL_FastQuery(gH_Db, sQuery)) {
		PrintToServer("Database Error: %s", sError);
		
		return false;
	}
	
	return true;
}

/**
* Adds a player to a gang
*
* @param client			Client ID
* @param gang			Gang ID
* @return				success if successful
* @error				Invalid client/gang ID or database connection
*/
stock bool SQL_AddToGang(int client, int gang) {
	char sQuery[255];
	char sID[64];
	GetClientAuthId(client, AuthId_Steam2, sID, sizeof(sID));
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	Format(sQuery, sizeof(sQuery), "UPDATE betterwarden_players SET GangID = %d WHERE SteamID LIKE '%s'", gang, sID);
	
	if(!SQL_FastQuery(gH_Db, sQuery)) {
		PrintToServer("Database Error: %s", sError);
		
		return false;
	}
	
	return true;
}

/**
* Fetch the owner of a gang
*
* @param gang			Gang ID
* @return				Client ID if successful, -1 if not
* @error				Invalid gang ID
*/
stock int SQL_GetGangOwner(int gang) {
	char sQuery[255];
	char sID[64]; // buffer
	
	char sError[255];
	gH_Db = SQL_Connect("betterwarden", true, sError, sizeof(sError));
	
	if(gH_Db != INVALID_HANDLE) {
		Format(sQuery, sizeof(sQuery), "SELECT Owner FROM betterwarden_gangs WHERE ID = %d", gang);
		
		DBResultSet SQL = SQL_Query(gH_Db, sQuery);
		
		if(SQL != null) {
			if(SQL_FetchRow(SQL)) {
				SQL_FetchString(SQL, 0, sID, sizeof(sID));
				for(int i = 1; i <= MaxClients; i++) {
					if(!IsValidClient(i, _, true))
						continue;
					char buff[64];
					GetClientAuthId(i, AuthId_Steam2, buff, sizeof(buff));
					
					if(!StrEqual(buff, sID, false))
						continue;
					
					CloseHandle(gH_Db);
					return i;
				}
				
				CloseHandle(gH_Db);
				return -1;
			}
		}
	}
	
	PrintToServer("Database error: %s", sError);
	
	return -1; // Owner not online or not found
}

/**
* Send a message to the gang chat
*
* @param gang			Gang ID
* @param message		Message to send (formatting rules)
* @error				Invalid gang ID
*/
native void SendToGang(int gang, const char[] message, any ...);

/**
* Sends a message from a client to the gang chat
*
* @param sender			Client ID of the sender
* @param gang			Gang ID
* @param message		Message to send (formatting rules)
* @error				Invalid gang ID
*/
native void CPrintToChatGang(int sender, int gang, const char[] message, any ...);

/**
* Called once a gang is created by a client.
*
* @param client			Client ID of the creating entity
* @param gang			Name of the created gang
*/
forward void OnGangCreated(int client, const char[] gang);

/**
* Called once a gang leader disbands their gang.
* 
* @param gang			Name of the gang
*/
forward void OnGangDisbanded(const char[] gang);

/**
* Called once a gang invite was created and sent to another entity.
* 
* @param client			Client ID of the invited entity
* @param admin			Client ID of the admin entity
* @param gang			Name of the gang
*/
forward void OnGangInvite(int client, int admin, const char[] gang);

/**
* Called once an invite has been accepted.
* 
* @param client			Client ID of the entity
* @param gang			ID of the gang
*/
forward void OnGangInviteAccepted(int client, int gang);

/**
* Called once an invite has been declined.
* 
* @param client			Client ID of the entity
* @param gang			Name of the gang
*/
forward void OnGangInviteDeclined(int client, const char[] gang);

/**
* Called once a gang-chat message was sent
* 
* @param client			Client ID of the entity
* @param msg			Message sent to chat
* @param gang			Name of the gang
*/
forward void OnGangChat(int client, const char[] msg, const char[] gang);

/**
* Called once a gang member leaves their gang.
*
* @param client			Client ID of the entity
* @param gang			Name of the gang
*/
forward void OnGangMemberLeave(int client, const char[] gang);

/**
* Called once a gang member has been kicked from their gang.
* 
* @param client			Client ID of the entity
* @param admin			Client ID of the admin issuing the command
* @param gang			Name of the gang
*/
forward void OnGangMemberKicked(int client, int admin, const char[] gang);

public SharedPlugin __pl_bwgangs = 
{
	name = "gangs",
	file = "gangs.smx",
#if defined REQUIRE_PLUGIN
	required = 1,
#else
	required = 0,
#endif
};
 
#if !defined REQUIRE_PLUGIN
public void __pl_bwgangs_SetNTVOptional()
{
	//MarkNativeAsOptional("");
}
#endif