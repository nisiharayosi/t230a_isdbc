/*How to Use
$ gcc ts_ch_dect.c -o ts_ch_dect
$ ./ts_ch_dect ./ch41.ts | nkf --ic=CP932 > ch.txt
--OPTION--
    -t : 出力リストチャンネルタイプ [ type: *** ] / デフォルト: BS
    -c : dvbチャンネルリストを使用してチャンネル番号出力 [ channel: '***' ] / デフォルト: 周波数出力 [channel: ***.***MHz]
*/
#define TS_PACKET_SIZE 188
#define NIT_TABLE_ID 0x40
//SDT_TABLE_ID : 0x42(自チャンネル名)
//SDT_TABLE_ID : 0x42(他チャンネル名)
//SDT_TABLE_ID : 0x00(全チャンネル名)
#define SDT_TABLE_ID 0x00

#define def_ch_type "BS\0\0"

#include <stdio.h>

struct _channels {
    unsigned int t_id;
    unsigned int n_id;
    unsigned int s_id;
    unsigned char c_name[30];
    unsigned char c_type;
    unsigned char mcl_type[4];
    float c_freq;
    unsigned char c_num[5];
    unsigned int modul;
    float symbR;
};
typedef struct _channels channels_t;


void arib_conv(unsigned char* str, unsigned int str_len, unsigned char* r_str){
    /* ARIB 8単位符号をシフトJISに変換して出力する */
    int g[4],gl,gr,ss,jc,jh,jl,k,s_cont;
    unsigned long long ac;
	g[0]=0x0142;	/* G0=漢字(2バイトGセット) */
	g[1]=0x004a;	/* G1=英数(1バイトGセット) */
	g[2]=0x0030;	/* G2=平仮名(1バイトGセット) */
	g[3]=0x0031;	/* G3=片仮名(1バイトGセット) */
	gl=0;		/* GL=G0 */
	gr=2;		/* GR=G2 */
	ss=-1;		/* シングルシフト解除 */
	ac=0;
	k=0;
    s_cont=0;
	while(k<str_len) {
		jc=0;
		ac=(ac<<8)|str[k];
		k++;
		if(ac==0x0e) {				/* LS1 (GL=G1) */
			gl=1;
			ac=0;
		}
		else if(ac==0x0f) {			/* LS0 (GL=G0) */
			gl=0;
			ac=0;
		}
		else if(ac==0x19) {			/* SS2 (GL=G2 シングルシフト) */
			ss=gl;
			gl=2;
			ac=0;
		}
		else if(ac==0x1d) {			/* SS3 (GL=G3 シングルシフト) */
			ss=gl;
			gl=3;
			ac=0;
		}
		else if(ac==0x1b6e) {			/* LS2 (GL=G2) */
			gl=2;
			ac=0;
		}
		else if(ac==0x1b6f) {			/* LS3 (GL=G3) */
			gl=3;
			ac=0;
		}
		else if(ac==0x1b7c) {			/* LS3R (GR=G3) */
			gr=3;
			ac=0;
		}
		else if(ac==0x1b7d) {			/* LS2R (GR=G2) */
			gr=2;
			ac=0;
		}
		else if(ac==0x1b7e) {			/* LS1R (GR=G1) */
			gr=1;
			ac=0;
		}
		else if(ac==0x20) {			/* SP (空白) */
			jc=0x2121;
			ac=0;
		}
		else if(ac==0x89) {			/* MSZ (中型サイズ) */
			ac=0;
		}
		else if(ac==0x8a) {			/* NSZ (標準サイズ) */
			ac=0;
		}
		else if(ac>=0x21 && ac<=0x7e) {		/* GL (1バイト文字) */
			if(g[gl]==0x004a) jc=0x2300+ac;		/* 英数 */
			else if(g[gl]==0x0030) jc=0x2400+ac;	/* 平仮名 */
			else if(g[gl]==0x0031) jc=0x2500+ac;	/* 片仮名 */
			if((g[gl]&0x0f00)==0x0000) ac=0;
		}
		else if(ac>=0x2100 && ac<=0x7eff) {	/* GL (2バイト文字) */
			if((ac&0xff)>=0x21 && (ac&0xff)<=0x7e && g[gl]==0x0142) jc=ac;
			ac=0;
		}
		else if(ac>=0xa1 && ac<=0xfe) {		/* GR (1バイト文字) */
			if(g[gr]==0x004a) jc=0x2300+(ac&0x7f);		/* 英数 */
			else if(g[gr]==0x0030) jc=0x2400+(ac&0x7f);	/* 平仮名 */
			else if(g[gr]==0x0031) jc=0x2500+(ac&0x7f);	/* 片仮名 */
			if((g[gr]&0x0f00)==0x0000) ac=0;
		}
		else if(ac>=0xa100 && ac<=0xfeff) {	/* GR (2バイト文字) */
			if((ac&0xff)>=0xa1 && (ac&0xff)<=0xfe && g[gr]==0x0142) jc=ac&0x7f7f;
			ac=0;
		}
		/* 文字表示 */
		if(jc>0) {
			/* 文字変換 */
			if(jc==0x2321) jc=0x212a;	/* エクスクラメンション */
			else if(jc==0x2328) jc=0x214a;	/* カッコ始め */
			else if(jc==0x2329) jc=0x214b;	/* カッコ閉じ */
			else if(jc==0x232d) jc=0x215d;	/* マイナス */
			else if(jc==0x232f) jc=0x213f;	/* スラッシュ */
			else if(jc==0x2479) jc=0x213c;	/* 長音記号(平仮名) */
			else if(jc==0x2579) jc=0x213c;	/* 長音記号(片仮名) */
			else if(jc==0x247e) jc=0x2126;	/* 中黒記号(平仮名) */
			else if(jc==0x257e) jc=0x2126;	/* 中黒記号(平
			/* シフトJIS変換 */
			jh=(((jc/256)-0x21)/2)+0x81;
			if(jh>=0xa0) jh=jh+0x40;
			if(((jc/256)%2)==0) {
				jl=(jc%256)-0x21+0x9f;
			}
			else {
				jl=(jc%256)-0x21+0x40;
				if(jl>=0x7f) jl=jl+0x01;
				}
			//printf("%c%c",jh,jl);
            r_str[s_cont*2] = jh;
            r_str[s_cont*2+1] = jl;
            s_cont++;
			/* シングルシフト解除 */
			if(ss>=0) {
				gl=ss;
				ss=-1;
			}
		}
		if(ac>0xff) ac=0;
	}
} 

int main(int argc,char *argv[]) {
    unsigned int ts_file = 1, isdbc_ch_file = 0, channels_type = 0;

    FILE* fp;
    unsigned int p_cont, p_id, ts_id, ts_data_len, ts_data_pos, st_data_len, st_data_pos, cur_data_len, cur_data_pos, ch_cont = 0;
    unsigned char p_buff[TS_PACKET_SIZE], a_buff[TS_PACKET_SIZE*10], b_buff[TS_PACKET_SIZE],pusi, tid, ss, cc, af_c, af_l, b_seek;

    channels_t ch_list[20];

    //引数、オプション判別処理
    if(argc>1){
        for(int i = 1; i < argc; i++){
            if(argv[i][0] == '-'){
                switch(argv[i][1]){
                    case 't':
                        channels_type = ++i;
                    break;
                    case 'c':
                        isdbc_ch_file = ++i;
                    break;
                    default:
                    break;
                }
            }
            else{
                ts_file = i;
            }
        }
    }
    else{//help
        printf("Usage: ts_ch_dect [OPTION...] <ts file>\n\n\t-c\tuse channels list file(output channel number)\n\t\tex) channel: C60\n\t\tdefault) channel: '***.***'MHz(frequency)\n\t-t\tset output channel type\n\t\tex) -t SKY => \"type: SKY\"\n\t\tdefault) type: BS(Max 3 character = 3 Byte)\n\n\n example)\n$ ./ts_ch_dect -c dvbv5_channels_isdbc.conf -t SKY ./ch40.ts | nkf --ic=CP932 > ch.txt\n& cat ch.txt\n### Channels List ###\n\n- name: BS-TBS\n  type: sky\n  channel: 'C49'\n  isDisabled: false\n  serviceId: 161\n- name: unkoun\n~~~~~~~~~~~~~~~~\n\n");
        return 0;
    }

    fp = fopen( argv[ts_file], "rb" );
    if(fp){
        //以下NIT解析
        while(1){//NITパケット先頭探索、読み込み
            if((TS_PACKET_SIZE == fread(p_buff, 1, TS_PACKET_SIZE, fp)) && (p_buff[0] == 0x47)){
                p_id = (0x1F & p_buff[1]) << 8 | p_buff[2];
                pusi = (0x40 & p_buff[1]) >> 6;
                af_c = (0x30 & p_buff[3]) >> 4;
                if(af_c != 0x01){
                    af_l = p_buff[4];
                }
                else{
                    af_l = 0;
                }
                ss = p_buff[4+af_l];
                tid = p_buff[5+af_l+ss];
                if(0x0010 == p_id && 0x01 == pusi && NIT_TABLE_ID == tid){
                    //printf("PID :0x%04x (Network Information Table)\n", p_id);
                    af_c = (0x30 & p_buff[3]) >> 4;
                    cc = 0x0f & p_buff[3];
                    //printf("PSUI ::0x%02x\nCC :0x%02x", pusi, cc);
                    for(int i = 0; i < (TS_PACKET_SIZE); i++){
                        a_buff[i] = p_buff[i];
                    }
                    p_cont = TS_PACKET_SIZE;
                    af_l = 0;

                    /*//以下デバッグ用バイナリ出力
                    for(int i = 0; i < TS_PACKET_SIZE; i++){
                        if(((i - 0) % 8) == 0){
                            printf("\n");
                            printf("%03u  : ",i+1);
                        }
                        printf("0x%02x ", p_buff[i]);
                    }
                    printf("\n\n");
                    *///以上デバッグ用バイナリ出力

                    break;
                }
            }
            else{
                break;
            }
            fseek(fp, TS_PACKET_SIZE, SEEK_CUR);
        }
        while(1){//NITパケット分割探索、読み込み
            if((TS_PACKET_SIZE == fread(p_buff, 1, TS_PACKET_SIZE, fp)) && (p_buff[0] == 0x47)){
                p_id = (0x1F & p_buff[1]) << 8 | p_buff[2];
                pusi = (0x40 & p_buff[1]) >> 6;
                if(0x0010 == p_id && 0x00 == pusi){
                    //printf("Success!!\n");
                    af_c = (0x30 & p_buff[3]) >> 4;
                    cc = 0x0f & p_buff[3];
                    //printf("PSUI ::0x%02x\nCC :0x%02x", pusi, cc);
                    if(af_c != 0x01){
                        af_l = p_buff[4];
                    }
                    for(int i = 0; i < (TS_PACKET_SIZE - 4 - af_l); i++){
                        a_buff[i+p_cont] = p_buff[i+4+af_l];
                    }
                    p_cont += TS_PACKET_SIZE - 4 - af_l;;
                    af_l = 0;

                    /*//以下デバッグ用バイナリ出力
                    for(int i = 0; i < TS_PACKET_SIZE; i++){
                        if(((i - 0) % 8) == 0){
                            printf("\n");
                            printf("%03u  : ",i+1);
                        }
                        printf("0x%02x ", p_buff[i]);
                    }
                    printf("\n\n");
                    *///以上デバッグ用バイナリ出力

                }
                else if(0x0010 == p_id && 0x01 == pusi){
                    break;
                }

            }
            else{
                printf("ERROR :Read Error!\n\n");
                break;
            }
            fseek(fp, TS_PACKET_SIZE, SEEK_CUR);
        }
        fseek(fp, 0, SEEK_SET);

        af_c = (0x30 & a_buff[3]) >> 4;
        if(af_c != 0x01){
            af_l = a_buff[4];
        }
        else{
            af_l = 0;
        }
        ss = a_buff[4+af_l];
        b_seek = af_l + ss;
        //payload_start
        tid = a_buff[5+b_seek];
        //printf("TABLE  :0x%02x\n", tid);
        //printf("SC-Len :%u\n", (((0x0f & a_buff[6+b_seek]) << 8) | a_buff[7+b_seek]));
        //printf("NID :0x%04x\n", ((a_buff[8+b_seek] << 8) | a_buff[9+b_seek]));
        //printf("CN-flg :0x%02x\n", (0x01 & a_buff[10+b_seek]));
        //printf("SC-Num :0x%02x\n", a_buff[11+b_seek]);
        //printf("SC-End-Num :0x%02x\n", a_buff[12+b_seek]);
        cur_data_len = (((0x0f & a_buff[13+b_seek]) << 8) | a_buff[14+b_seek]);
        //printf("Nv-Len :0x%04x\n", cur_data_len);
        switch(a_buff[15+b_seek]){//ネットワーク記述子領域1
            case 0x40:{
                unsigned char str_tmp[30];
                //arib_conv(&a_buff[17+b_seek],a_buff[16+b_seek],str_tmp);
                //printf("\n");
                }
                break;

            default:
                //printf("N1_Are_ID :0x%02x\n", a_buff[15+b_seek]);
                break;
        }
        cur_data_pos = 15 + b_seek + cur_data_len;
        cur_data_len = (((0x0f & a_buff[cur_data_pos]) << 8) | a_buff[cur_data_pos+1]);
        //printf("\nTS_lo-Len :0x%04x  (%u)\n\n", cur_data_len, cur_data_len);
        ts_data_len = cur_data_len;
        cur_data_pos++;
        ts_data_pos = cur_data_pos;
        cur_data_pos++;
        while(1){
            if(ts_data_len < (cur_data_pos - ts_data_pos) || cur_data_pos >= p_cont || cur_data_pos >= sizeof(a_buff)) break;
            //printf("TSID :0x%04x\n", ((a_buff[cur_data_pos] << 8) | a_buff[cur_data_pos+1]));
            ch_list[ch_cont].t_id = ((a_buff[cur_data_pos] << 8) | a_buff[cur_data_pos+1]);
            //printf("oNID :0x%04x\n", ((a_buff[cur_data_pos+2] << 8) | a_buff[cur_data_pos+3]));
            ch_list[ch_cont].n_id = ((a_buff[cur_data_pos+2] << 8) | a_buff[cur_data_pos+3]);
            st_data_len = (((0x0f & a_buff[cur_data_pos+4]) << 8) | a_buff[cur_data_pos+5]);
            //printf("TSvL :0x%04x\n", st_data_len);
            cur_data_pos += 6;
            st_data_pos = cur_data_pos;
            while(1){
                if(st_data_len <= (cur_data_pos - st_data_pos) || cur_data_pos >= p_cont || cur_data_pos >= sizeof(a_buff)) break;
                switch(a_buff[cur_data_pos]){//ネットワーク記述子領域2
                    case 0x41://サービスリスト記述子
                        //printf("Tag :0x%02x\n\n",a_buff[cur_data_pos]);
                        //printf("SID : 0x%04x\nType: 0x%02x\n", ((a_buff[cur_data_pos+2] << 8) | a_buff[cur_data_pos+3]), a_buff[cur_data_pos+4]);
                        ch_list[ch_cont].c_type = a_buff[cur_data_pos+4];
                        cur_data_pos += a_buff[cur_data_pos+1] + 2;
                    break;
                    case 0x44:{//有線分配システム記述子
                        //printf("Tag :0x%02x\n",a_buff[cur_data_pos]);
                        //printf("Len :0x%02x\n",a_buff[cur_data_pos+1]);
                        float freq_bcd = (((0xf0&a_buff[cur_data_pos+2])>>4)*1000)+((0x0f&a_buff[cur_data_pos+2])*100)+(((0xf0&a_buff[cur_data_pos+3])>>4)*10)+(0x0f&a_buff[cur_data_pos+3])+(((0xf0&a_buff[cur_data_pos+4])>>4)/(float)10)+((0x0f&a_buff[cur_data_pos+4])/(float)100)+(((0xf0&a_buff[cur_data_pos+5])>>4)/(float)1000)+((0x0f&a_buff[cur_data_pos+5])/(float)10000);
                        //printf("freq :%6.3f\n\n", freq_bcd);
                        ch_list[ch_cont].c_freq = freq_bcd;
                        //printf("TSMF :0x%02x\n", (0xf0&a_buff[cur_data_pos+7])>>4);
                        //printf("Modu :0x%02x", a_buff[cur_data_pos+8]);
                        switch(a_buff[cur_data_pos+8]){
                            case 0x03:
                                //printf("(64 QAM)\n");
                                ch_list[ch_cont].modul = 64;
                            break;
                            case 0x05:
                                //printf("(256 QAM)\n");
                                ch_list[ch_cont].modul = 256;
                            break;
                        }
                        freq_bcd = (((0xf0&a_buff[cur_data_pos+9])>>4)*100)+((0x0f&a_buff[cur_data_pos+9])*10)+((0xf0&a_buff[cur_data_pos+10])>>4)+((0x0f&a_buff[cur_data_pos+10])/(float)10)+(((0xf0&a_buff[cur_data_pos+11])>>4)/(float)100)+((0x0f&a_buff[cur_data_pos+11])/(float)1000)+(((0xf0&a_buff[cur_data_pos+12])>>4)/(float)10000);
                        //printf("SymR :%4.3f\n\n", freq_bcd);
                        ch_list[ch_cont].symbR = freq_bcd;
                        ch_list[ch_cont].s_id = 0x0000;
                        //NITで取得できない情報を初期化
                        unsigned char n_tmp[] = "unkoun";
                        for(int i = 0; i < sizeof(ch_list[ch_cont].c_name)-1; i++){
                            if(i<(sizeof(n_tmp)-1)){
                                ch_list[ch_cont].c_name[i] = n_tmp[i];
                            }
                            else{
                                ch_list[ch_cont].c_name[i] = *"";
                            }
                        }
                        cur_data_pos += a_buff[cur_data_pos+1] + 2;
                        ch_cont++;
                        }
                    break;
                    default:
                        //printf("Tag :0x%02x\n\n",a_buff[cur_data_pos]);
                        cur_data_pos += a_buff[cur_data_pos+1] + 2;
                    break;
                }
            }
        }

        //以下SDT解析
        int sdi_lo = 1;
        while(sdi_lo){
            while(1){//SDTパケット先頭探索、読み込み
                if((TS_PACKET_SIZE == fread(p_buff, 1, TS_PACKET_SIZE, fp)) && (p_buff[0] == 0x47)){
                    p_id = (0x1F & p_buff[1]) << 8 | p_buff[2];
                    pusi = (0x40 & p_buff[1]) >> 6;
                    af_c = (0x30 & p_buff[3]) >> 4;
                    if(af_c != 0x01){
                        af_l = p_buff[4];
                    }
                    else{
                        af_l = 0;
                    }
                    ss = p_buff[4+af_l];
                    tid = p_buff[5+af_l+ss];
                    if(0x0011 == p_id && 0x01 == pusi && (SDT_TABLE_ID == 0x00 || SDT_TABLE_ID == tid)){
                        //printf("PID :0x%04x (Service Description Table)\n", p_id);
                        af_c = (0x30 & p_buff[3]) >> 4;
                        cc = 0x0f & p_buff[3];
                        //printf("PSUI ::0x%02x\nCC :0x%02x", pusi, cc);
                        for(int i = 0; i < (TS_PACKET_SIZE); i++){
                            a_buff[i] = p_buff[i];
                        }
                        p_cont = TS_PACKET_SIZE;
                        af_l = 0;

                        /*//以下デバッグ用バイナリ出力
                        for(int i = 0; i < TS_PACKET_SIZE; i++){
                            if(((i - 0) % 8) == 0){
                                printf("\n");
                                printf("%03u  : ",i+1);
                            }
                            printf("0x%02x ", p_buff[i]);
                        }
                        printf("\n\n");
                        *///以上デバッグ用バイナリ出力

                        break;
                    }
                }
                else{
                    if(0 == fread(p_buff, 1, TS_PACKET_SIZE, fp))sdi_lo = 0;
                    break;
                }
                fseek(fp, TS_PACKET_SIZE, SEEK_CUR);
            }
            if(sdi_lo == 0) break;
            while(1){//SDTパケット分割探索、読み込み
                if((TS_PACKET_SIZE == fread(p_buff, 1, TS_PACKET_SIZE, fp)) && (p_buff[0] == 0x47)){
                    p_id = (0x1F & p_buff[1]) << 8 | p_buff[2];
                    pusi = (0x40 & p_buff[1]) >> 6;
                    if(0x0011 == p_id && 0x00 == pusi){
                        //printf("Success!!\n");
                        af_c = (0x30 & p_buff[3]) >> 4;
                        cc = 0x0f & p_buff[3];
                        //printf("PSUI ::0x%02x\nCC :0x%02x", pusi, cc);
                        if(af_c != 0x01){
                            af_l = p_buff[4];
                        }
                        for(int i = 0; i < (TS_PACKET_SIZE - 4 - af_l); i++){
                            a_buff[i+p_cont] = p_buff[i+4+af_l];
                        }
                        p_cont += TS_PACKET_SIZE - 4 - af_l;;
                        af_l = 0;

                        /*//以下デバッグ用バイナリ出力
                        for(int i = 0; i < TS_PACKET_SIZE; i++){
                            if(((i - 0) % 8) == 0){
                                printf("\n");
                                printf("%03u  : ",i+1);
                            }
                            printf("0x%02x ", p_buff[i]);
                        }
                        printf("\n\n");
                        *///以上デバッグ用バイナリ出力

                    }
                    else if(0x0011 == p_id && 0x01 == pusi){
                        break;
                    }

                }
                else{
                    break;
                }
                fseek(fp, TS_PACKET_SIZE, SEEK_CUR);
            }

            af_c = (0x30 & a_buff[3]) >> 4;
            if(af_c != 0x01){
                af_l = a_buff[4];
            }
            else{
                af_l = 0;
            }
            ss = a_buff[4+af_l];
            b_seek = af_l + ss;
            //payload_start
            tid = a_buff[5+b_seek];
            //printf("TABLE  :0x%02x\n", tid);
            if(SDT_TABLE_ID == 0x00 || SDT_TABLE_ID == tid){
                //printf("SC-Len :%u\n", (((0x0f & a_buff[6+b_seek]) << 8) | a_buff[7+b_seek]));
                ts_id = ((a_buff[8+b_seek] << 8) | a_buff[9+b_seek]);
                //printf("TSID :0x%04x\n", ts_id);
                for(int i = 0; i < ch_cont; i++){
                    if(ch_list[i].t_id == ts_id){
                        /*
                        for(int t = 0; t < p_cont; t++){
                            if(((t - 0) % 8) == 0){
                                printf("\n");
                                printf("%03u  : ",t+1);
                            }
                            printf("0x%02x ", a_buff[t]);
                        }
                        printf("\n\n");
                        */
                        //printf("CN-flg :0x%02x\n", (0x01 & a_buff[10+b_seek]));
                        //printf("SC-Num :0x%02x\n", a_buff[11+b_seek]);
                        //printf("SC-End-Num :0x%02x\n", a_buff[12+b_seek]);
                        //printf("oNID :0x%04x\n", (((0x0f & a_buff[13+b_seek]) << 8) | a_buff[14+b_seek]));
                        cur_data_pos = 16 + b_seek;
                        while(1){
                            if(cur_data_pos >= p_cont || cur_data_pos >= sizeof(a_buff)) break;
                            //printf("SID  :0x%04x\n", ((a_buff[cur_data_pos] << 8) | a_buff[cur_data_pos+1]));
                            ch_list[i].s_id = ((a_buff[cur_data_pos] << 8) | a_buff[cur_data_pos+1]);
                            st_data_len = (((0x0f & a_buff[cur_data_pos+3]) << 8) | a_buff[cur_data_pos+4]);
                            //printf("SV-Len :0x%04x\n", st_data_len);
                            cur_data_pos += 5;
                            st_data_pos = cur_data_pos;
                            while(1){
                                if(st_data_len <= (cur_data_pos - st_data_pos) || cur_data_pos >= p_cont || cur_data_pos >= sizeof(a_buff)) break;
                                switch(a_buff[cur_data_pos]){
                                    case 0x48://サービス記述子
                                        //printf("Tag :0x%02x\n\n",a_buff[cur_data_pos]);
                                        //printf("BC   :");
                                        //arib_conv(&a_buff[cur_data_pos+4],a_buff[cur_data_pos+3], NULL);
                                        cur_data_pos = cur_data_pos+4 + a_buff[cur_data_pos+3];
                                        //printf("\nSV   :");
                                        arib_conv(&a_buff[cur_data_pos+1],a_buff[cur_data_pos],ch_list[i].c_name);
                                        cur_data_pos = cur_data_pos+1 + a_buff[cur_data_pos];
                                        //printf("\n\n");
                                    break;
                                    default:
                                        //printf("Tag :0x%02x\n\n",a_buff[cur_data_pos]);
                                        cur_data_pos += p_buff[cur_data_pos+1] + 2;
                                    break;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
        fclose(fp);
    }
    else{
        printf("ERROR :ts file open error!\n");
        return 1;
    }
    //printf("\nDiscover %d channels.\n",ch_cont);
    int name_cont = ch_cont;
    unsigned char n_tmp[] = "unkoun";
    int str_check;
    for(int i = 0; i < ch_cont; i++){
        str_check = 1;
        for(int t = 0; t < sizeof(n_tmp); t++){
            if(ch_list[i].c_name[t] != n_tmp[t]) str_check = 0;
        }
        if(str_check == 1) name_cont--;
    }
    unsigned char mc_type[] = def_ch_type;
    for(int i = 0; i < ch_cont; i++){
        if(ch_list[i].c_type == 0x01){
            if(channels_type != 0){
                for(int t = 0; t < sizeof(ch_list[i].mcl_type); t++){
                    if(sizeof(argv[channels_type]) > t){
                        ch_list[i].mcl_type[t] = argv[channels_type][t];
                    }
                    else{
                        ch_list[i].mcl_type[t] = *"";
                    }
                }
            }
            else{
                for(int t = 0; t < sizeof(ch_list[i].mcl_type); t++){
                    if(sizeof(mc_type) > t){
                        ch_list[i].mcl_type[t] = mc_type[t];
                    }
                    else{
                        ch_list[i].mcl_type[t] = *"";
                    }
                }
            }
        }
    }
    int get_ch_number = 0;
    float get_ch_freq = 0;
    cur_data_len = 0;
    if(isdbc_ch_file != 0){
        fp = fopen( argv[isdbc_ch_file], "rb" );
        if(fp){
            while(1){
                if(0 != (ts_data_len = fread(p_buff, 1, TS_PACKET_SIZE, fp))){
                    for(int t = 0; t < ts_data_len; t++){
                        //printf("%c", p_buff[t]);
                    }
                    for(int i = 0; i < ts_data_len; i++){
                        if(i >= 30 && get_ch_number == 0){
                            cur_data_len = i+1;
                            break;
                        }
                        if(p_buff[i] == '['&& get_ch_number == 0){
                            i++;
                            for(int t = 0; t < sizeof(mc_type); t++){
                                if(p_buff[i] == 'C'||p_buff[i] == '1'||p_buff[i] == '2'||p_buff[i] == '3'||p_buff[i] == '4'||p_buff[i] == '5'||p_buff[i] == '6'||p_buff[i] == '7'||p_buff[i] == '8'||p_buff[i] == '9'||p_buff[i] == '0'){
                                    mc_type[t] = p_buff[i];
                                }
                                else{
                                    mc_type[t] = *"";
                                }
                                i++;
                            }
                            printf("%s\n",mc_type);
                            get_ch_number = 1;
                        }
                        if(p_buff[i] == 'F'&&p_buff[i+1] == 'R'&&p_buff[i+2] == 'E'&&p_buff[i+3] == 'Q'){
                            get_ch_freq = 0;
                            i += 10;
                            unsigned int k = 0;
                            for(int t = 0; t < 14; t++){
                                if(p_buff[i] == '1'||p_buff[i] == '2'||p_buff[i] == '3'||p_buff[i] == '4'||p_buff[i] == '5'||p_buff[i] == '6'||p_buff[i] == '7'||p_buff[i] == '8'||p_buff[i] == '9'||p_buff[i] == '0'){
                                    get_ch_freq = (get_ch_freq * 10) + (unsigned int)(0x0f&p_buff[i]);
                                }
                                else if(p_buff[i] == '\n'){
                                    get_ch_freq /= 1000000;
                                    break;
                                }
                                i++;
                            }
                            printf("%f\n",get_ch_freq);
                            get_ch_number = 2;
                        }
                        if(p_buff[i] == '[' && get_ch_number > 0){
                            printf("NEXT...\n");
                            cur_data_len = i;
                            break;
                        }
                        if(i == ts_data_len-1){
                            cur_data_len = ts_data_len;
                            fseek(fp, TS_PACKET_SIZE, SEEK_CUR);
                        }
                    }
                    if(get_ch_number > 1){
                        for(int i = 0; i < ch_cont; i++){
                            if(get_ch_freq == ch_list[i].c_freq){
                                printf("Success!\n");
                                for(int t = 0; t < sizeof(ch_list[i].c_num); t++){
                                    ch_list[i].c_num[t] = mc_type[t];
                                }
                            }
                        }
                    }
                }
                else{
                    break;
                }
                get_ch_number = 0;
                fseek(fp, -(TS_PACKET_SIZE-cur_data_len), SEEK_CUR);
            }
        }
        else{
            printf("ERROR :channels list file open error! => '%s'\n\n",argv[isdbc_ch_file]);
        }
    }
    //printf("\nName Discover %d channels.\n",name_cont);
    printf("### Channels List ###\n\n");
    for(int i = 0; i < ch_cont; i++){
        if(ch_list[i].c_type == 0x01){
            //printf("[No.%03d]\nTSID :0x%04x\noNID :0x%04x\nSID  :0x%04x\nNAME :%s\nType :0x%02x\nFreq :%6.3f MHz\nModl :%d QAM\nSymR :%4.3f MHz\n\n", i+1, ch_list[i].t_id, ch_list[i].n_id, ch_list[i].s_id, ch_list[i].c_name, ch_list[i].c_type, ch_list[i].c_freq, ch_list[i].modul, ch_list[i].symbR);
            if(isdbc_ch_file == 0){
                //Mirakurun channels list Format(channel: freq)
                printf("- name: %s\n  type: %s\n  channel: '%6.3f'MHz\n  isDisabled: false\n  serviceId: %d\n\n", ch_list[i].c_name, ch_list[i].mcl_type, ch_list[i].c_freq, ch_list[i].s_id);
            }
            else{
                //Mirakurun channels list Format(channel: number)
                printf("- name: %s\n  type: %s\n  channel: '%s'\n  isDisabled: false\n  serviceId: %d\n", ch_list[i].c_name, ch_list[i].mcl_type, ch_list[i].c_num, ch_list[i].s_id);
            }
        }
    }
}