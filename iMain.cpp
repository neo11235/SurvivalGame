#include "iGraphics.h"
#include <stdio.h>
#include <math.h>
using namespace std;
/************************settings*****************************/
typedef struct color_{
    double r,g,b;
}Color;

#define food 1
#define pred 2
#define new_game            1
#define game_pause          2
#define game_on             3
#define game_over           4
#define game_high_score     5
#define game_how_to_play    6
#define start_new_game      7
#define game_new_high_score 8
#define hflag0 0
#define hflag1 1
#define hflag2 2
#define dead_ 0
#define alive_ 1
#define mxx(a,b) ((a)>(b)?(a):(b))
#define mnn(a,b) ((a)<(b)?(a):(b))
#define dabs(x) (x<0.0?-x:x)
const int Max_initial_radius=30;
const int Max_speed=1000;
const int Max_radius=100;
const double growth_factor=.35;
int game_state=new_game;
const int player_initial_radius=20;
const double Max_player_speed=2500.0/400.0;
const double dspeed=.01;
const double pres=pow(10.0,-6.00);
int e_time=0;
int score=0;
int max_e_time,max_score;//load at the start of the game
int g_state=0;
char best_scorar[100];
char best_survivor[100];
int best_score;
int best_survive_time;
int input_state=0;
int input_cursor=0;
int hflag;
char tmp_array[1000]="Enter your nick name(no space):";
char score_ara[1000]="Score:";
char survive_ara[1000]="Time:";
char iscore[100];
char iSurvive[100];
Color background_color;
Color food_color;
Color player_eye_color;
Color carr[13];
/**********************************/
Color get_rgb(int hex){
    Color c;
    c.b=hex&(~((~0)<<8));
    c.g=(hex>>8)&(~((~0)<<8));
    c.r=(hex>>16)&(~((~0)<<8));
    return c;
}
double state_angle[6];
/***********************************************************/
void iFilledCircle2(double x, double y, double r, int slices,double th1,double th2)
{
    iFilledCircle(x,y,r,slices);
    iSetColor(background_color.r,background_color.g,background_color.b);
    double t, dt, x1,y1, xp, yp;
    dt = 2*M_PI/slices;
    xp = x+r*cos(th1);
    yp = y+r*sin(th1);
    glBegin(GL_POLYGON);
    for(t = th1; t <= th2; t+=dt)
    {
        x1 = x + r * cos(t);
        y1 = y + r * sin(t);

        glVertex2f(xp, yp);
        xp = x1;
        yp = y1;
    }
    glVertex2f(x,y);
    glEnd();
}
/*****************mathematical function************************/
double distance1(double x,double y){
    return sqrt((x*x)+(y*y));
}
/*****************mathematical function************************/
/******************fileIO********************************/
void load_high_score(void){
    FILE* fp=fopen("hscore.txt","r");
    if(fp==NULL){
        best_scorar[0]='\0';
        best_survivor[0]='\0';
        best_score=0;
        best_survive_time=0;
        return;
    }
    fscanf(fp,"%d %s",&best_score,best_scorar);
    fscanf(fp,"%d %s",&best_survive_time,best_survivor);
    fclose(fp);
    return;
}
void flush_current_score(void){
    FILE* fp=fopen("hscore.txt","w");
    if(fp==NULL){
        return;
    }
    fprintf(fp,"%d %s\n%d %s\n",best_score,best_scorar,best_survive_time,best_survivor);
    fclose(fp);
    return;
}
/************************************************************/
typedef struct entity_{//in order to implement AI we will just need to change Information structure and brain function
    double px,py,radius;
    Color color;
    double speed,dir_angle;
    double dx,dy;

    int flag1;//dead or alive
    int flag2;//for checking collition
    int flag3;//is it food of entity
    int state;//for animation
    int state2;//for brain
    int brain;
}Entity;
Entity tmp_ent;
void change_direction_to_point(Entity* ent,double px,double py);
void check_and_Resolve_collision(void);
typedef struct the_world{//most prbably it will change and entity rock and food will be kept in suitable data structure for optimization
    Entity entity[30];//entity 0 represent the player whose brain function will be set to NULL;
    int num_of_alive;//initially 30
    int entity_lower_bound;
    int percent_of_food;
    int w_width,w_height;//same as screen width and height

}World;
World world;
typedef struct{
    double px,py,width,height;
}rectangle;
rectangle rec_new_game,rec_high_score,rec_about,rec_exit1,rec_exit2,rec_continue,rec_main_menu,rec_main_menu_2,rec_exit3,rec_return,rec_return2;
struct player_control{
    double px,py,radius;
    double inner_px,inner_py,inner_radius;
}PLC;
void save_current_score(void){//if its highest
    flush_current_score();
    return;
}
void my_exit(int code){
    if(code==0){
        save_current_score();
    }
    else printf("Fatal error. Code %d\n",code);
    exit(0);
}
Color color_function(double radius){
    int id=(int)((12.00*radius)/Max_radius);
    id=min(id,12);
    return carr[id];
}

void draw_entity(Entity* ent){
    if(ent->flag3==food){
        iSetColor(ent->color.r,ent->color.g,ent->color.b);
        iFilledCircle(ent->px,ent->py,ent->radius,100);
    }
    else{
        iSetColor(ent->color.r,ent->color.g,ent->color.b);
        iFilledCircle2(ent->px,ent->py,ent->radius,150,\
                       ent->dir_angle-state_angle[abs(ent->state)],ent->dir_angle+state_angle[abs(ent->state)]);
        //ent->state++;
        //if(ent->state==6)ent->state=-6;
    }
    return;
}
int entity_regenerator(Entity* ent){
    int id=rand()%4,pb1=50;
    double incr=(double)(rand()%4);
    if(e_time>50){
        pb1=min(e_time,70);
    }
    if((rand()%100)>pb1)incr*=-1.0;
    ent->radius=world.entity[0].radius+incr;
    ent->color=color_function(ent->radius);
    ent->flag1=alive_;
    ent->flag3=pred;
    if(e_time>60){
        ent->speed=((double)Max_speed+(double)(rand()%500))/400.0;
    }
    else{
        ent->speed=((double)(rand()%(Max_speed-10))+100.0)/400.0;
    }

    if(id==0){
        ent->py=(double)(rand()%world.w_height);
        ent->px=-ent->radius;
    }
    else if(id==1){
        ent->px=(double)(rand()%world.w_width);
        ent->py=-ent->radius;
    }
    else if(id==2){
        ent->py=(double)(rand()%world.w_height);
        ent->px=(double)world.w_width+ent->radius;
    }
    else{
        ent->px=(double)(rand()%world.w_width);
        ent->py=(double)world.w_height+ent->radius;
    }

    change_direction_to_point(ent,700,rand()%900);
    ent->state2=(int)((2.7*ent->radius)/ent->speed);
    ent->state2*=-1;
    ent->state=0;
    world.num_of_alive++;
    return 0;
}
int entity_constructor(Entity* ent){

    ent->px=(double)(rand()%(world.w_width-2*Max_initial_radius))+Max_initial_radius;
    ent->py=(double)(rand()%(world.w_height-2*Max_initial_radius))+Max_initial_radius;
    ent->radius=(double)(rand()%Max_initial_radius)+5.00;
    if((rand()%100)<world.percent_of_food){
        ent->flag3=food;
        ent->flag1=alive_;
        ent->color=food_color;
        return 1;
    }
    ent->flag3=pred;
    ent->flag1=alive_;
    ent->speed=(double)(rand()%Max_speed)/400.0;
    ent->dir_angle=(((double)(rand()%360))/180.0)*M_PI;
    ent->dx=ent->speed*cos(ent->dir_angle);
    ent->dy=ent->speed*sin(ent->dir_angle);
    ent->color=color_function(ent->radius);
    ent->state=0;
    ent->state2=0;
    return 0;

}
void player_constructor(Entity* ent,int ps=0){
    ent->dir_angle=M_PI/3.00;
    ent->speed=0;
    ent->dx=0;
    ent->dy=0;
    ent->px=ent->py=200.0;
    ent->radius=player_initial_radius;
    ent->color=color_function(ent->radius);
    ent->color.r=255;
    ent->color.g=ent->color.g=0;
    ent->flag1=alive_;
    ent->flag3=pred;
    ent->state=0;
    ent->state2=0;

}
double get_dir_angle(double dx,double dy){
    double dir_angle;
    if(dabs(dx)<pres){
        if(dy>=0)return M_PI/2.0;
        else return (3.0/2.0)*M_PI;
    }
    else if(dabs(dy)<pres){
        if(dx<0)return M_PI;
        else return 0;
    }
    if(dx>0){
        if(dy>0){
            dir_angle=atan(dy/dx);
        }
        else{
            dir_angle=2*M_PI-atan(abs(dy/dx));
        }
    }
    else{
        if(dy>=0){
            dir_angle=M_PI-atan(abs(dy/dx));
        }
        else{
            dir_angle=M_PI+atan(abs(dy/dx));
        }
    }
    return dir_angle;
}
void change_direction_to_point(Entity* ent,double px,double py){
    double tdx,tdy,t;
    tdx=px-ent->px;
    tdy=py-ent->py;
    t=distance1(tdx,tdy);
    ent->dx=tdx/t;
    ent->dy=tdy/t;
    ent->dir_angle=get_dir_angle(ent->dx,ent->dy);
    return;

}
void entity_brain1(Entity* ent,int id){
    double tdx,tdy,t;
    if(ent->state2>=0){
    if((ent->px<=ent->radius)||(ent->py<=ent->radius)||(ent->px>=(world.w_width-ent->radius))||(ent->py>=(world.w_height-ent->radius))){
        change_direction_to_point(ent,700,(double)(rand()%900));
        return;
    }
    }
    ent->state2++;
    if(ent->state2==5){
        ent->state2=0;
        int closest_id=-1,i;
        double dmn=10000000,td;
        for(i=0;i<30;i++){
        if(world.entity[i].flag1&&i!=id&&((world.entity[i].px+world.entity[i].radius)>3&&(world.entity[i].py+world.entity[i].radius)>3&&\
                                   (world.entity[i].px)<(world.w_width+world.entity[i].radius-3)&&world.entity[i].py<(world.w_height+world.entity[i].radius-3))){
            td=distance1(world.entity[id].px-world.entity[i].px,world.entity[id].py-world.entity[i].py);
            if(td<dmn){
                dmn=td;
                closest_id=i;
            }
        }
        }
        if(closest_id!=-1){
            if(world.entity[closest_id].flag3==food||world.entity[closest_id].radius<ent->radius){
                change_direction_to_point(ent,world.entity[closest_id].px,world.entity[closest_id].py);
            }
            else{
                if(dmn<=100){
                    change_direction_to_point(ent,2.0*ent->px-world.entity[closest_id].px,2.0*ent->py-world.entity[closest_id].py);
                }
            }
        }

    }

    return;
}
void entity_brain2(Entity* ent,int id){
    entity_brain1(ent,id);
    return;
}
void init(int number_of_ent=30){
    int i;

    background_color.r=193;
    background_color.g=255;
    background_color.b=149;
    player_eye_color.r=player_eye_color.g=player_eye_color.b=0;
    food_color.r=0;
    food_color.g=153;
    food_color.b=0;

    carr[0]=get_rgb(0x009900);
    carr[1]=get_rgb(0x00cc00);
    carr[2]=get_rgb(0x00ff00);
    carr[3]=get_rgb(0x33ff33);
    carr[4]=get_rgb(0x66ff66);
    carr[5]=get_rgb(0x80ff00);
    carr[6]=get_rgb(0xffff66);
    carr[7]=get_rgb(0xffb266);
    carr[8]=get_rgb(0xff9999);
    carr[9]=get_rgb(0xff6666);
    carr[10]=get_rgb(0xff3333);
    carr[11]=get_rgb(0xff0000);
    carr[12]=get_rgb(0xcc0000);

    world.w_height=950;
    world.w_width=1500;

    world.percent_of_food=5;

    world.num_of_alive=number_of_ent;
    world.entity_lower_bound=11;

    player_constructor(&world.entity[0]);
    world.entity[0].brain=0;

    rec_new_game.px=650;
    rec_new_game.py=700;
    rec_new_game.width=rec_high_score.width=rec_about.width=rec_exit1.width=rec_continue.width=rec_main_menu.width=rec_return.width=rec_return2.width=200;
    rec_new_game.height=rec_high_score.height=rec_about.height=rec_exit1.height=rec_continue.height=rec_main_menu.height=rec_return.height=rec_return2.height=50;

    rec_high_score.px=rec_continue.px=650;
    rec_high_score.py=rec_continue.py=630;
    rec_return2.px=500;
    rec_return2.py=350;

    rec_about.px=rec_main_menu.px=650;
    rec_about.py=rec_main_menu.py=560;

    rec_exit1.px=650;
    rec_exit1.py=490;

    rec_return.px=600;
    rec_return.py=520;


    PLC.px=PLC.inner_px=tmp_ent.px=world.w_width/2.0;
    PLC.py=PLC.inner_py=tmp_ent.py=100.0;
    PLC.radius=80;
    PLC.inner_radius=20;

    for(i=1;i<number_of_ent;i++){
        entity_constructor(&world.entity[i]);
        if(world.entity[i].flag3==pred){
            world.entity[i].brain=rand()%2;

        }
    }
    for(i=0;i<6;i++){
        state_angle[i]=((double)i*M_PI)/24.00;
    }
    return;
}
void draw_player(Entity* ent){
    iSetColor(ent->color.r,ent->color.g,ent->color.b);
    iFilledCircle2(ent->px,ent->py,ent->radius,150,\
                    ent->dir_angle-state_angle[abs(ent->state)],ent->dir_angle+state_angle[abs(ent->state)]);
    iSetColor(player_eye_color.r,player_eye_color.g,player_eye_color.b);
    iFilledCircle(ent->px+(ent->radius*cos(M_PI/3.0+ent->dir_angle))/2.0,ent->py+\
                  (ent->radius*sin(M_PI/3.0+ent->dir_angle))/2.0,ent->radius/4.6,150);
    //ent->state++;
    //if(ent->state==6)ent->state=-6;
}
void draw_control(void){
    iSetColor(255,0,0);
    iCircle(PLC.px,PLC.py,PLC.radius,200);
    iFilledCircle(PLC.inner_px,PLC.inner_py,PLC.inner_radius,100);
}
void draw_pause_state(void){
    iSetColor(255,0,0);
    iText(rec_new_game.px+4,rec_new_game.py+14,"GAME PAUSED!!!",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_continue.px,rec_continue.py,rec_continue.width,rec_continue.height);
    iText(rec_continue.px+37,rec_continue.py+14,"CONTINUE",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_main_menu.px,rec_main_menu.py,rec_main_menu.width,rec_main_menu.height);
    iText(rec_main_menu.px+35,rec_main_menu.py+14,"MAIN MENU",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_exit1.px,rec_exit1.py,rec_exit1.width,rec_exit1.height);
    iText(rec_exit1.px+70,rec_exit1.py+14,"EXIT",GLUT_BITMAP_TIMES_ROMAN_24);
}
void draw_new_game(void){
    iSetColor(background_color.r,background_color.g,background_color.b);
    iFilledRectangle(0,0,world.w_width,world.w_height);
    //iShowBMP(0,0,"PROJECT.bmp");
    iSetColor(255,0,0);
    iText(rec_new_game.px-40,rec_new_game.py+100,"EAT RUN AND SURVIVE",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_new_game.px,rec_new_game.py,rec_new_game.width,rec_new_game.height);
    iText(rec_new_game.px+32,rec_new_game.py+14,"NEW GAME",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_high_score.px,rec_high_score.py,rec_high_score.width,rec_high_score.height);
    iText(rec_high_score.px+32,rec_high_score.py+14,"HIGH SCORE",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_about.px,rec_about.py,rec_about.width,rec_about.height);
    iText(rec_about.px+20,rec_about.py+14,"INSTRUCTION",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_exit1.px,rec_exit1.py,rec_exit1.width,rec_exit1.height);
    iText(rec_exit1.px+70,rec_exit1.py+14,"EXIT",GLUT_BITMAP_TIMES_ROMAN_24);
    return;
}
void draw_high_score(void){
    iSetColor(background_color.r,background_color.g,background_color.b);
    iFilledRectangle(0,0,world.w_width,world.w_height);
    char datat[10],dataS[10],dataout1[100]="Best Survival Time:",dataout2[100]="Best Score:",dataout3[200]="Best Survivor:",dataout4[200]="Best Scorar:";
    strcat(dataout3,best_survivor);
    strcat(dataout4,best_scorar);
    itoa(best_survive_time,datat,10);
    itoa(best_score,dataS,10);
    strcat(dataout1,datat);
    strcat(dataout1,"s");
    strcat(dataout2,dataS);
    iSetColor(255,0,0);
    iText(600,700,dataout1,GLUT_BITMAP_TIMES_ROMAN_24);
    iText(600,670,dataout2,GLUT_BITMAP_TIMES_ROMAN_24);
    iText(600,640,dataout3,GLUT_BITMAP_TIMES_ROMAN_24);
    iText(600,610,dataout4,GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_return.px,rec_return.py,rec_return.width,rec_return.height);
    iText(rec_return.px+50,rec_return.py+15,"GO BACK",GLUT_BITMAP_TIMES_ROMAN_24);
}
void draw_about(void){
    iSetColor(background_color.r,background_color.g,background_color.b);
    iFilledRectangle(0,0,world.w_width,world.w_height);
    iSetColor(255,0,0);
    iText(500,650,"1.Click left to change your speed and direction",GLUT_BITMAP_TIMES_ROMAN_24);
    iText(500,600,"2.Click right to pause the game",GLUT_BITMAP_TIMES_ROMAN_24);
    iText(500,550,"3.You can eat any entity smaller than you and grow in size",GLUT_BITMAP_TIMES_ROMAN_24);
    iText(500,500,"4.You can eat food of any size",GLUT_BITMAP_TIMES_ROMAN_24);
    iText(500,450,"5.If you get eaten the game will be over",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_return2.px,rec_return2.py,rec_return2.width,rec_return2.height);
    iText(rec_return2.px+50,rec_return2.py+15,"GO BACK",GLUT_BITMAP_TIMES_ROMAN_24);
    iText(500,200,"Created by Sajib and Hasan",GLUT_BITMAP_TIMES_ROMAN_24);
}
void draw_new_high_score(void){
    iSetColor(background_color.r,background_color.g,background_color.b);
    iFilledRectangle(0,0,world.w_width,world.w_height);
    iSetColor(255,0,0);
    if(input_state<4){
        iText(650,600,"HIGH SCORE!!!",GLUT_BITMAP_TIMES_ROMAN_24);
    }
    else{
        char* str;
        if(hflag==hflag0||hflag==hflag1)str=best_survivor;
        else str=best_scorar;
        int i=0,j=31;
        for(;i<input_cursor;i++,j++){
            tmp_array[j]=str[i];
        }
        if(input_state%2){
            tmp_array[j]='|';
            j++;
        }
        tmp_array[j]='\0';
        iText(600,600,tmp_array,GLUT_BITMAP_TIMES_ROMAN_24);
        /****/
        iSetColor(255,0,0);
        itoa(e_time,iSurvive,10);
        itoa(score,iscore,10);
        strcat(survive_ara,iSurvive);
        strcat(score_ara,iscore);
        iText(600,630,survive_ara,GLUT_BITMAP_TIMES_ROMAN_24);
        iText(600,650,score_ara,GLUT_BITMAP_TIMES_ROMAN_24);
        iText(600,670,"HIGH SCORE!!!",GLUT_BITMAP_TIMES_ROMAN_24);
        survive_ara[5]='\0';
        score_ara[6]='\0';
        /***/
    }
}
void draw_game_over(void){
    iSetColor(background_color.r,background_color.g,background_color.b);
    iFilledRectangle(0,0,world.w_width,world.w_height);
    iSetColor(255,0,0);
        iText(rec_main_menu.px,rec_main_menu.py+130,"Game Over!!!",GLUT_BITMAP_TIMES_ROMAN_24);

        itoa(e_time,iSurvive,10);
        itoa(score,iscore,10);
        strcat(survive_ara,iSurvive);
        strcat(score_ara,iscore);
        iText(rec_main_menu.px,rec_main_menu.py+100,score_ara,GLUT_BITMAP_TIMES_ROMAN_24);
        iText(rec_main_menu.px,rec_main_menu.py+70,survive_ara,GLUT_BITMAP_TIMES_ROMAN_24);
        survive_ara[5]='\0';
        score_ara[6]='\0';

    iRectangle(rec_main_menu.px,rec_main_menu.py,rec_main_menu.width,rec_main_menu.height);
    iText(rec_main_menu.px+35,rec_main_menu.py+14,"MAIN MENU",GLUT_BITMAP_TIMES_ROMAN_24);
    iRectangle(rec_exit1.px,rec_exit1.py,rec_exit1.width,rec_exit1.height);
    iText(rec_exit1.px+70,rec_exit1.py+14,"EXIT",GLUT_BITMAP_TIMES_ROMAN_24);
}
void iDraw()
{
    int i;
    iClear();
    switch(game_state){
    case new_game:
        draw_new_game();//complete
        break;
    case game_high_score:
        draw_high_score();
        break;
    case game_how_to_play:
        draw_about();
        break;
    case game_on:
        iSetColor(background_color.r,background_color.g,background_color.b);
        iFilledRectangle(0,0,world.w_width,world.w_height);
        draw_player(&world.entity[0]);
        for(i=1;i<30;i++){
            if(world.entity[i].flag1)draw_entity(&world.entity[i]);
        }
        draw_control();
        //draw score and elapsed time here later
        iSetColor(255,0,0);
        itoa(e_time,iSurvive,10);
        itoa(score,iscore,10);
        strcat(survive_ara,iSurvive);
        strcat(score_ara,iscore);
        iText(1300,20,survive_ara,GLUT_BITMAP_TIMES_ROMAN_24);
        iText(1300,50,score_ara,GLUT_BITMAP_TIMES_ROMAN_24);
        survive_ara[5]='\0';
        score_ara[6]='\0';
        break;
    case game_new_high_score:
        draw_new_high_score();
        break;
    case start_new_game:
        iSetColor(background_color.r,background_color.g,background_color.b);
        iFilledRectangle(0,0,world.w_width,world.w_height);
        /*******************************************/
        draw_player(&world.entity[0]);
        for(i=1;i<30;i++){
            if(world.entity[i].flag1)draw_entity(&world.entity[i]);
        }
        draw_control();

        /*********************************************/

        iSetColor(255,0,0);
        if(g_state<2)iText(680,500,"READY?",GLUT_BITMAP_TIMES_ROMAN_24);
        else if(g_state<3)iText(680,500,"SET?",GLUT_BITMAP_TIMES_ROMAN_24);
        else if(g_state<4)iText(680,500,"GAME ON!!!",GLUT_BITMAP_TIMES_ROMAN_24);
        else{
            iPauseTimer(2);
            iResumeTimer(0);
            iResumeTimer(1);
            game_state=game_on;
        }
        break;
    case game_pause:
        iSetColor(background_color.r,background_color.g,background_color.b);
        iFilledRectangle(0,0,world.w_width,world.w_height);
        draw_player(&world.entity[0]);
        for(i=1;i<30;i++){
            if(world.entity[i].flag1)draw_entity(&world.entity[i]);
        }
        draw_control();
        draw_pause_state();
        break;
    default://game over
        draw_game_over();
        break;

    }
}

int my_mouse_state=GLUT_UP;
void iMouseMove(int mx, int my){
    if(my_mouse_state==GLUT_DOWN){
        if(game_state==game_on){
            change_direction_to_point(&tmp_ent,(double)mx,(double)my);
            world.entity[0].dir_angle=tmp_ent.dir_angle;
            world.entity[0].dx=tmp_ent.dx;
            world.entity[0].dy=tmp_ent.dy;

            double d=distance1((double)mx-PLC.px,(double)my-PLC.py);
            if(d<PLC.radius){
                PLC.inner_px=(double)mx;
                PLC.inner_py=(double)my;
                world.entity[0].speed=(d/PLC.radius)*Max_player_speed;
            }
            else{
                PLC.inner_px=750.0+PLC.radius*cos(world.entity[0].dir_angle);
                PLC.inner_py=100.0+PLC.radius*sin(world.entity[0].dir_angle);
                world.entity[0].speed=Max_player_speed;
            }
            my_mouse_state=GLUT_DOWN;

        }
    }
}
void check_and_Resolve_collision2(void){//boundary must be solved by brain function
    int i,j;

    double d;
    for(i=0;i<30;i++){
    if(world.entity[i].flag1){
        for(j=i+1;j<30;j++){
        if(world.entity[j].flag1){
            d=distance1(world.entity[i].px-world.entity[j].px,world.entity[i].py-world.entity[j].py);
            if((d<=(world.entity[i].radius+world.entity[j].radius))){//there is a bug//logical error
                if(world.entity[i].flag3==food){
                    world.entity[i].flag1=dead_;
                    world.entity[j].radius=min((double)Max_radius,distance1(world.entity[j].radius,growth_factor*world.entity[i].radius));
                    if(j!=0&&world.entity[j].flag3==pred)world.entity[j].color=color_function(world.entity[j].radius);
                    if(j==0)score++;
                }
                else if(world.entity[j].flag3==food){
                    world.entity[j].flag1=dead_;
                    world.entity[i].radius=min((double)Max_radius,distance1(world.entity[j].radius*growth_factor,world.entity[i].radius));
                    if(i!=0&&world.entity[i].flag3==pred)world.entity[i].color=color_function(world.entity[i].radius);
                    if(i==0)score++;
                }
                else{
                    world.num_of_alive--;
                    if(i==0){
                        world.entity[j].flag1=dead_;
                        world.entity[i].radius=min((double)Max_radius,distance1(world.entity[j].radius*growth_factor,world.entity[i].radius));
                    }
                    else if(world.entity[i].radius<world.entity[j].radius&&world.entity[j].flag3!=food){
                        world.entity[i].flag1=dead_;
                        world.entity[j].radius=min((double)Max_radius,distance1(world.entity[j].radius,growth_factor*world.entity[i].radius));
                        if(j!=0)world.entity[j].color=color_function(world.entity[j].radius);
                        if(j==0)score++;
                        break;
                    }
                    else{
                        world.entity[j].flag1=dead_;
                        world.entity[i].radius=min((double)Max_radius,distance1(world.entity[j].radius*growth_factor,world.entity[i].radius));
                        if(i!=0)world.entity[i].color=color_function(world.entity[i].radius);
                        if(i==0)score++;
                    }
                }
            }
        }
        }
    }
    }

    return;
}
void iMouse(int button, int state, int mx, int my)
{
    if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
    {
        if(game_state==game_on){
            change_direction_to_point(&tmp_ent,(double)mx,(double)my);
            world.entity[0].dir_angle=tmp_ent.dir_angle;
            world.entity[0].dx=tmp_ent.dx;
            world.entity[0].dy=tmp_ent.dy;
            double d=distance1((double)mx-PLC.px,(double)my-PLC.py);
            if(d<PLC.radius){
                PLC.inner_px=(double)mx;
                PLC.inner_py=(double)my;
                world.entity[0].speed=(d/PLC.radius)*Max_player_speed;
            }
            else{
                PLC.inner_px=750.0+PLC.radius*cos(world.entity[0].dir_angle);
                PLC.inner_py=100.0+PLC.radius*sin(world.entity[0].dir_angle);
                world.entity[0].speed=Max_player_speed;
            }
            my_mouse_state=GLUT_DOWN;
        }
        else if(game_state==new_game){
            if(mx>=rec_new_game.px&&(mx<=(rec_new_game.px+rec_new_game.width))&&my>=rec_new_game.py&&(my<=(rec_new_game.py+rec_new_game.height))){
                game_state=start_new_game;
                init(30);
                check_and_Resolve_collision2();
                e_time=score=0;
                iResumeTimer(2);
                g_state=0;
            }
            else if(mx>=rec_high_score.px&&(mx<=(rec_high_score.px+rec_high_score.width))&&my>=rec_high_score.py&&my<=(rec_high_score.py+rec_high_score.height)){
                game_state=game_high_score;
            }
            else if(mx>=rec_about.px&&(mx<=(rec_about.px+rec_about.width))&&my>=rec_about.py&&(my<=(rec_about.py+rec_about.height))){
                game_state=game_how_to_play;
            }
            else if(mx>=rec_exit1.px&&(mx<=(rec_exit1.px+rec_exit1.width))&&my>=rec_exit1.py&&(my<=(rec_exit1.py+rec_exit1.height))){
                my_exit(0);
            }
        }
        else if(game_state==game_pause){
            if(mx>=rec_continue.px&&(mx<=(rec_continue.px+rec_continue.width))&&my>=rec_continue.py&&my<=(rec_continue.py+rec_continue.height)){
                iResumeTimer(0);
                iResumeTimer(1);
                game_state=game_on;
            }
            else if(mx>=rec_main_menu.px&&(mx<=(rec_main_menu.px+rec_main_menu.width))&&my>=rec_main_menu.py&&(my<=(rec_main_menu.py+rec_main_menu.height))){
                game_state=new_game;
            }
            else if(mx>=rec_exit1.px&&(mx<=(rec_exit1.px+rec_exit1.width))&&my>=rec_exit1.py&&(my<=(rec_exit1.py+rec_exit1.height))){
                my_exit(0);
            }
        }
        else if(game_state==game_high_score){
            if(mx>=rec_return.px&&(mx<=(rec_return.px+rec_return.width))&&my>=rec_return.py&&(my<=(rec_return.py+rec_return.height))){
                game_state=new_game;
            }
        }
        else if(game_state==game_how_to_play){
            if(mx>=rec_return2.px&&(mx<=(rec_return2.px+rec_return2.width))&&my>=rec_return2.py&&(my<=(rec_return2.py+rec_return2.height))){
                game_state=new_game;
            }
        }
        else if(game_state==game_over){
            if(mx>=rec_main_menu.px&&(mx<=(rec_main_menu.px+rec_main_menu.width))&&my>=rec_main_menu.py&&(my<=(rec_main_menu.py+rec_main_menu.height))){
                game_state=new_game;
            }
            else if(mx>=rec_exit1.px&&(mx<=(rec_exit1.px+rec_exit1.width))&&my>=rec_exit1.py&&(my<=(rec_exit1.py+rec_exit1.height))){
                my_exit(0);
            }
        }
        else if(game_state==game_high_score){

        }
    }
    if(button==GLUT_LEFT_BUTTON&&state==GLUT_UP){
        if(game_state==game_on){
            world.entity[0].speed=0;
            PLC.inner_px=750.0;
            PLC.inner_py=100.0;
            my_mouse_state=GLUT_UP;
        }
    }
    if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN){
        if(game_state==game_on){
            game_state=game_pause;
            iPauseTimer(0);
            iPauseTimer(1);
        }
    }

}

/*
	function iKeyboard() is called whenever the user hits a key in keyboard.
	key- holds the ASCII value of the key pressed.
*/
void iKeyboard(unsigned char key)
{
    if(game_state!=game_new_high_score){
        if(key == 'q')exit(0);
        if(key=='p'){
            game_state=game_pause;
            iPauseTimer(0);
            iPauseTimer(1);
        }
        if(key=='r'){
            game_state=game_on;
            iResumeTimer(0);
            iResumeTimer(1);
        }
    }

    else{//game_state==game_new_high_score player giving input
        if(!((key>='A'&&key<='Z')||(key>='a'&&key<='z')||key==13||key==8))return;
        if(key==8){
            input_cursor=mxx(0,input_cursor-1);
            return;
        }
        if(hflag==hflag0){
            best_scorar[input_cursor]=key;
            best_survivor[input_cursor]=key;
        }
        else if(hflag==hflag1){
            best_survivor[input_cursor]=key;
        }
        else{
            best_scorar[input_cursor]=key;
        }
        if(key==13){
            if(hflag==hflag0){
                best_scorar[input_cursor]='\0';
                best_survivor[input_cursor]='\0';
            }
            else if(hflag==hflag1){
                best_survivor[input_cursor]='\0';
            }
            else{
                best_scorar[input_cursor]='\0';
            }
            flush_current_score();
            iPauseTimer(3);
            game_state=new_game;
        }
        input_cursor++;

    }

}

/*
	function iSpecialKeyboard() is called whenver user hits special keys like-
	function keys, home, end, pg up, pg down, arraows etc. you have to use
	appropriate constants to detect them. A list is:
	GLUT_KEY_F1, GLUT_KEY_F2, GLUT_KEY_F3, GLUT_KEY_F4, GLUT_KEY_F5, GLUT_KEY_F6,
	GLUT_KEY_F7, GLUT_KEY_F8, GLUT_KEY_F9, GLUT_KEY_F10, GLUT_KEY_F11, GLUT_KEY_F12,
	GLUT_KEY_LEFT, GLUT_KEY_UP, GLUT_KEY_RIGHT, GLUT_KEY_DOWN, GLUT_KEY_PAGE UP,
	GLUT_KEY_PAGE DOWN, GLUT_KEY_HOME, GLUT_KEY_END, GLUT_KEY_INSERT
*/
void iSpecialKeyboard(unsigned char key)
{

    if(key == GLUT_KEY_END)
    {
        exit(0);
    }
    if(key==GLUT_KEY_LEFT)world.entity[0].px-=4;//for diagnostic purpose
    if(key==GLUT_KEY_RIGHT)world.entity[0].px+=4;
    if(key==GLUT_KEY_DOWN)world.entity[0].py-=4;
    if(key==GLUT_KEY_UP)world.entity[0].py+=4;
    //place your codes for other keys here
}
void check_and_Resolve_collision(void){//boundary must be solved by brain function
    int i,j;

    double d;
    for(i=0;i<30;i++){
    if(world.entity[i].flag1){
        for(j=i+1;j<30;j++){
        if(world.entity[j].flag1){
            d=distance1(world.entity[i].px-world.entity[j].px,world.entity[i].py-world.entity[j].py);
            if((d<=(world.entity[i].radius+world.entity[j].radius))){//there is a bug//logical error fixed
                if(world.entity[i].flag3==food){
                    world.entity[i].flag1=dead_;
                    world.entity[j].radius=min((double)Max_radius,distance1(world.entity[j].radius,growth_factor*world.entity[i].radius));
                    if(j!=0&&world.entity[j].flag3==pred)world.entity[j].color=color_function(world.entity[j].radius);
                    if(j==0)score++;
                }
                else if(world.entity[j].flag3==food){
                    world.entity[j].flag1=dead_;
                    world.entity[i].radius=min((double)Max_radius,distance1(world.entity[j].radius*growth_factor,world.entity[i].radius));
                    if(i!=0&&world.entity[i].flag3==pred)world.entity[i].color=color_function(world.entity[i].radius);
                    if(i==0)score++;
                }
                else{
                    world.num_of_alive--;
                    if(world.entity[i].radius<world.entity[j].radius&&world.entity[j].flag3!=food){
                        world.entity[i].flag1=dead_;
                        world.entity[j].radius=min((double)Max_radius,distance1(world.entity[j].radius,growth_factor*world.entity[i].radius));
                        if(j!=0)world.entity[j].color=color_function(world.entity[j].radius);
                        if(j==0)score++;
                        break;
                    }
                    else{
                        world.entity[j].flag1=dead_;
                        world.entity[i].radius=min((double)Max_radius,distance1(world.entity[j].radius*growth_factor,world.entity[i].radius));
                        if(i!=0)world.entity[i].color=color_function(world.entity[i].radius);
                        if(i==0)score++;
                    }
                }
            }
        }
        }
    }
    }

    return;
}
void player_boundary_check(void){
    if(world.entity[0].px<=world.entity[0].radius){
        world.entity[0].px=world.entity[0].radius+.01;
        world.entity[0].dx=0;
    }
    if(world.entity[0].px>=((double)world.w_width-world.entity[0].radius)){
        world.entity[0].px=((double)world.w_width-world.entity[0].radius-.01);
        world.entity[0].dx=0;
    }
    if(world.entity[0].py<=world.entity[0].radius){
        world.entity[0].py=world.entity[0].radius+.01;
        world.entity[0].dy=0;
    }
    if(world.entity[0].py>=((double)world.w_height-world.entity[0].radius)){
        world.entity[0].py=((double)world.w_height-world.entity[0].radius-.01);
        world.entity[0].dy=0;
    }
}
void movement(void){
    int i;
    for(i=0;i<30;i++){
        if(world.entity[i].flag1){
            world.entity[i].state++;
            if(world.entity[i].state==6)world.entity[i].state=-6;
        }
    }
    for(i=0;i<30;i++){
    if(world.entity[i].flag1){
        if(world.entity[i].flag3==pred){
            world.entity[i].px+=world.entity[i].speed*world.entity[i].dx;
            world.entity[i].py+=world.entity[i].speed*world.entity[i].dy;
        }
    }
    }
    check_and_Resolve_collision();
    if(!world.entity[0].flag1){
        if(e_time>best_survive_time){
            best_survive_time=e_time;
            if(score>best_score){
                hflag=hflag0;
                best_score=score;
            }
            else hflag=hflag1;
            game_state=game_new_high_score;
            input_cursor=0;
            input_state=0;
            iResumeTimer(3);
        }
        else if(score>best_score){
            best_score=score;
            hflag=hflag2;
            game_state=game_new_high_score;
            input_cursor=0;
            input_state=0;
            iResumeTimer(3);
        }
        else{
            game_state=game_over;
        }
        iPauseTimer(0);
        iPauseTimer(1);
    }
    for(i=1;i<30;i++){
        if(world.entity[i].flag1){
            if(world.entity[i].brain)entity_brain1(&world.entity[i],i);
            else entity_brain2(&world.entity[i],i);
        }
    }
    player_boundary_check();
    if(world.num_of_alive<world.entity_lower_bound){
        int j=rand()%5;
        for(i=1;i<30;i++){
            if(!world.entity[i].flag1){
                entity_regenerator(&world.entity[i]);
                j--;
            }
            if(j<=0)break;
        }
        check_and_Resolve_collision();

    }
    return;
}
void elapsed_time(void){
    e_time++;
}
void fr_game_start(void){
    g_state++;
}
void for_name_input(void){
    input_state++;
}
int main()
{
    //place your own initialization codes here.
    init(30);
    check_and_Resolve_collision();
    load_high_score();
    iSetTimer(10,movement);
    iSetTimer(1000,elapsed_time);
    iSetTimer(1000,fr_game_start);
    iSetTimer(500,for_name_input);
    iPauseTimer(0);
    iPauseTimer(1);
    iPauseTimer(2);
    iPauseTimer(3);
    iInitialize(world.w_width, world.w_height, "Eat Run And Survive");

    return 0;
}
