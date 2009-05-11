#include "gameSettings.h"

void initGameSettings(s_gameSettings* gs){
	
	gs->ocarina = -1;
	gs->hooktype = -1;
	gs->language = -1;
	gs->video = -1;
	sprintf(gs->lastPlayed, "%s", "-1");
}

const char* _whitespace_cb(mxml_node_t *node, int where){

	const char *name;

	name = node->value.element.name;

	if (!strcmp(name, "wiicoverflow"))
		return ("\n");
	else if (!strcmp(name, "gamelist")){
		if (where == MXML_WS_BEFORE_OPEN)
			return ("\t");
		else if (where == MXML_WS_AFTER_OPEN)
			return ("\n");
		else if (where == MXML_WS_BEFORE_CLOSE)
			return ("\n\t");
		//else if (where == MXML_WS_AFTER_CLOSE)
		//	return ("\n");
	}
	else if(!strcmp(name, "game"))
	{
		if (where == MXML_WS_BEFORE_OPEN)
			return ("\t\t");
		//else if (where == MXML_WS_BEFORE_CLOSE)
		//	return ("\n");
		else if (where == MXML_WS_AFTER_CLOSE)
			return ("\n");
		else
			return (" ");
	}
	
	return (NULL);
}

bool getGameSettings(char* gameCode, s_gameSettings* gs){

	FILE *fp;
	mxml_node_t *xml;
	
	fp = fopen(USBLOADER_PATH "/gamelist.xml", "r");
  
	if(fp != NULL){
		
		xml = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
		fclose(fp);
		
		if(xml != NULL){
			
			mxml_node_t *parentNode;
			mxml_node_t *node;
			mxml_node_t *next_n;
			
			parentNode = mxmlFindElement(xml, xml, "wiicoverflow", NULL, NULL, MXML_DESCEND);
			
			if(parentNode != NULL){
				
				node = mxmlFindElement(xml, xml, "gamelist", NULL, NULL, MXML_DESCEND);
				
				if(node != NULL){
					
					for(next_n = mxmlFindElement(node, node, "game", NULL, NULL, MXML_DESCEND);
						next_n != NULL;
						next_n = mxmlFindElement(next_n, node, "game", NULL, NULL, MXML_DESCEND)
					)
					{
						if((strncmp(mxmlElementGetAttr(next_n, "id"), gameCode, 6) == 0)){
							
							if(mxmlElementGetAttr(next_n,"ocarina"))
								gs->ocarina = atoi(mxmlElementGetAttr(next_n,"ocarina"));
							else
								gs->ocarina = -1;
							if(mxmlElementGetAttr(next_n,"hooktype"))
								gs->hooktype = atoi(mxmlElementGetAttr(next_n,"hooktype"));
							else
								gs->hooktype = -1;
							if(mxmlElementGetAttr(next_n,"language"))
								gs->language = atoi(mxmlElementGetAttr(next_n,"language"));
							else
								gs->language = -1;
							if(mxmlElementGetAttr(next_n,"video"))
								gs->video = atoi(mxmlElementGetAttr(next_n,"video"));
							else
								gs->video = -1;
							if(mxmlElementGetAttr(next_n,"vipatch"))
								gs->vipatch = atof(mxmlElementGetAttr(next_n,"vipatch"));
							else
								gs->vipatch = -1;
							if(mxmlElementGetAttr(next_n,"lastplayed"))
								sprintf(gs->lastPlayed, "%s", mxmlElementGetAttr(next_n,"lastplayed"));
							else
								sprintf(gs->lastPlayed, "%s", "-1");
							
							return true;
						}
					}
				}
				
			}
		}
	}
	
	initGameSettings(gs); // put -1 everywhere if game is not found

	return false;
}

void succa(s_gameSettings* gs){

	gs->ocarina = 1;
	gs->hooktype = 2;
	gs->language = 3;
	gs->video = 4;
	gs->vipatch = 5;
}

void setGameSettings(char* gameCode, s_gameSettings* gs, int date){
	
	FILE *fp;
	mxml_node_t *xml;
	char temp[128];	
	bool found = false;
	
	fp = fopen(USBLOADER_PATH "/gamelist.xml", "r");
  
	if(fp != NULL){
		
		xml = mxmlLoadFile(NULL, fp, MXML_NO_CALLBACK);
		fclose(fp);
		
		if(xml != NULL){
			
			mxml_node_t *parentNode;
			mxml_node_t *node;
			mxml_node_t *next_n;
			
			parentNode = mxmlFindElement(xml, xml, "wiicoverflow", NULL, NULL, MXML_DESCEND);
			
			if(parentNode != NULL){
				
				node = mxmlFindElement(xml, xml, "gamelist", NULL, NULL, MXML_DESCEND);
				
				if(node != NULL){
					
					for(next_n = mxmlFindElement(node, node, "game", NULL, NULL, MXML_DESCEND);
						next_n != NULL;
						next_n = mxmlFindElement(next_n, node, "game", NULL, NULL, MXML_DESCEND)
					)
					{
						if((strncmp(mxmlElementGetAttr(next_n, "id"), gameCode, 6) == 0)){
							
							found = true;
							
							if(gs->ocarina != -1){
								sprintf(temp, "%d", gs->ocarina);
								mxmlElementSetAttr(next_n, "ocarina", temp);
							}
							if(gs->hooktype != -1){
								sprintf(temp, "%d", gs->hooktype);
								mxmlElementSetAttr(next_n, "hooktype", temp);
							}
							if(gs->language != -1){
								sprintf(temp, "%d", gs->language);
								mxmlElementSetAttr(next_n, "language", temp);
							}	
							if(gs->video != -1){
								sprintf(temp, "%d", gs->video);
								mxmlElementSetAttr(next_n, "video", temp);
							}	
							if(gs->vipatch != -1){
								sprintf(temp, "%d", gs->vipatch);
								mxmlElementSetAttr(next_n, "vipatch", temp);
							}
							
							if(date == 1){
								sysdate(temp);
								mxmlElementSetAttr(next_n, "lastplayed", temp);
							}
						}
					}
					
					if(!found){
						
						mxml_node_t *new_node;
						
						new_node = mxmlNewElement(node, "game");
						
						sprintf(temp, "%s", gameCode);
						mxmlElementSetAttr(new_node, "id", temp);
						
						sysdate(temp);
						mxmlElementSetAttr(new_node, "lastplayed", temp);
						
					}
					
					fp = fopen(USBLOADER_PATH "/gamelist.xml", "w");
					
					if(fp != NULL){
						mxmlSaveFile(xml, fp, _whitespace_cb);
						fclose(fp);
					}
					else
						WindowPrompt ("Error", "Can't save gamelist.xml", 0, &cancelButton);
				}
			}
		}
	}
}

void createEmptyGameSettingsFile(){
	
	mxml_node_t *xml;
	mxml_node_t *tree;
	mxml_node_t *node;
	
	FILE *fp;
	
	xml = mxmlNewXML("1.0");
	tree = mxmlNewElement(xml, "wiicoverflow");
	node = mxmlNewElement(tree, "gamelist");
	
	fp = fopen(USBLOADER_PATH "/gamelist.xml", "w");
        
	if(fp != NULL){
		mxmlSaveFile(xml, fp, _whitespace_cb);
		fclose(fp);
	}
	else
		WindowPrompt("ERROR", "Cannot create gamelist file!", 0, &okButton);
}
