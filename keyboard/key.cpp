//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
#include <stdio.h>

#include <sys/types.h>
#include <iostream>
#include <cx/base/string.h>
#include <cx/base/slist.h>
#include <cx/base/star.h>
#include <cx/base/hashmap.h>

#include <cx/editbuffer/editbuffer.h>
#include <cx/keyboard/keyboard.h>


int main(int argc, char **argv)
{

    CxKeyboard *keyboard = new CxKeyboard();
        
    while(1) {

        CxKeyAction keyAction = keyboard->getAction();

		printf("------------------------\n");
		printf("keyAction.definition() = [%s]\n", keyAction.definition().data() );
		printf("keyAction.tag()        = [%s]\n", keyAction.tag().data());
		printf("keyAction.actionType() = [%d]\n", keyAction.actionType() );

       
        switch (keyAction.actionType() ) {
                
            case CxKeyAction::LOWERCASE_ALPHA:
            case CxKeyAction::UPPERCASE_ALPHA:
            case CxKeyAction::NUMBER:
            case CxKeyAction::SYMBOL:
                {
					printf("%s\n", keyAction.tag().data());
					if (keyAction.tag() == "q") {
						delete keyboard;
						exit(0);
					}
				}
                break;
                
            case CxKeyAction::NEWLINE:
                printf("\n");
				break;

            case CxKeyAction::BACKSPACE:
                printf("%s\n", keyAction.tag().data());
                break;

            case CxKeyAction::TAB:
                printf("%s\n", keyAction.tag().data());
                break;
                
			case CxKeyAction::CURSOR:
                printf("%s\n", keyAction.tag().data());
				break;

			case CxKeyAction::CONTROL:

				if (keyAction.tag() == "K") {
					printf("delete right\n");
				}

				printf("control-%s\n", keyAction.tag().data());
				break;

			case CxKeyAction::COMMAND:
				printf("COMMAND-%s\n", keyAction.tag().data());
				break;

            default:
                break;
        }    
	}

	exit(0);
}




