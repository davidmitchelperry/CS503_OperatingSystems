/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

/************************************************************************/
/*									*/
/* main - main program for testing Xinu					*/
/*									*/
/************************************************************************/

void reader_func(char* name, pipid32 pip) {

	char buf[7500];
	int mylen;
	int length = 0;



	while(length < 7500) {

		if((mylen = pipread(pip, &buf[length], 7500 - length)) == SYSERR) {
			kprintf("reader error\n");
			break;
		}
		//kprintf("***mylength :%d***", mylen);
		length += mylen;
		//kprintf("***length :%d***", length);
	}
	kprintf("%s", buf);

	if(pipdisconnect(pip) == SYSERR) {
		kprintf("reader disc error\n");
	}
	sleep(1);
}

void writer_func(char* name, pipid32 pip) {
	
	//char buf[] = "davidiscool";
	char words[7500] = "WEST LAFAYETTE, Ind. - President Mitch Daniels began a new series of governance reports to the Purdue Board of Trustees on Friday (Feb. 21) with a look at institutional excellence. The report was the first of a series that will focus each meeting on one of the five areas of accountability chosen by the board and around which the president's contract is based. In April, the president will discuss fundraising; in May, he will discuss affordability, efficiency and student debt; in September, he will discuss student intellectual growth and achievement; and in December, he will discuss student success. In addition to Daniels' report, each meeting will feature updates on initiatives that are part of Purdue Moves and a report from an academic area. In Fridays report, Daniels previewed some preliminary themes recommended by a faculty committee on institutional excellence, including faculty members' recent awards and honors, their entrepreneurial activities and research productivity; and trends in admissions, including the percentage of top high school students who choose to attend Purdue. The number of Purdue faculty members who have received notable awards is encouraging, Daniels said. For example, more than 100 have won major early-career awards, and of those, 20 received their awards during 2013-14. In addition, 66 Purdue faculty members are fellows of the American Association for the Advancement of Science, 12 of whom received the honor this academic year. Faculty members' entrepreneurial activities also should be noted, Daniels said. For example, so far this fiscal year, faculty members have submitted 223 patent applications. The number of patent applications has increased steadily during the past six months. Another indicator is the value of faculty members' research awards. So far this fiscal year, faculty members have received about $231 million in research awards. That approaches the value of awards received between the same periods in fiscal year 2011 - the most lucrative year in the past 10. In the area of admissions, applications to Purdue have increased overall by 31 percent over last year. Some of that increase can be attributed to Purdue's recent acceptance of a nationwide common application. Specifically, domestic, out-of-state applications have increased 52 percent, which Daniels said is a rough indicator of Purdue's rising national reputation. Additionally, the number of resident and nonresident students who scored in the top 5 percent on the ACT or SAT has increased. Since fall 2009, that number as a percentage of the freshman class has risen more than 10 percentage points, to 31.5 percent. Presenting to trustees about Purdue's focus on investing in drug discovery were Philip Low, the Ralph C. Corley Distinguished Professor of Chemistry, and Timothy Ratliff, professor of comparative pathobiology and the Robert Wallace Miller Director of the Center for Cancer Research. Presenting about advancing plant science research were Jay Akridge, the Glenn W. Sample Dean of Purdue Agriculture, and Karen Plaut, senior associate dean for research and faculty affairs in the College of Agriculture. By investing in drug discovery, Purdue's researchers will be able to accelerate the rate at which they can move their work from the lab to commercialization and on to those who need it most, Low and Ratliff said. By advancing plant science research, Akridge and Plaut said Purdue's scientists will lead the world in translating their research to commercially important crops and eventually moving products through a pipeline for commercialization. For more information about these and other Purdue Moves, visit the initiatives' website. WEST LAFAYETTE, Ind. - The Purdue Board of Trustees on Friday (Feb. 21) approved naming the School of Civil Engineering for the Lyles family in recognition of a $15 million gift from the Lyles Foundation. The school will be known as the Lyles School of Civil Engineering. The gift will be used to broaden the school's capacities for learning and discovery and for generating solutions to critical global problems. \"The Lyles family has deep ties to Purdue and a long history of generous support for the university,\" said President Mitch Daniels. \"This latest of so many gifts fits uniquely into the university's action plan to increase the number of engineers we supply to our state and nation.\" In a statement, brothers Bill and Gerald Lyles said: \"We believe that a reinvigoration of the science, technology, engineering and math (STEM) disciplines is critical for our society. We are so proud of Purdue for taking the lead in STEM education in the United States. We are delighted that Purdue is expanding all aspects of its highly regarded programs to meet the national challenge to educate future engineers.\" They also noted that the School of Civil Engineering has been an integral part of their family and the industry in which they are involved. \"With proven abilities in experimental and computational techniques and our understanding of new and aging infrastructure, the School of Civil Engineering is addressing the grand challenges facing natural and built environments around the world,\" said Rao S. Govindaraju, the Bowen Engineering Head of Civil Engineering. \"The Lyles gift will enable us to amplify our impact through enhanced student learning and faculty research resulting in solutions that will lead to more sustainable and more resilient societies.\" The Lyles family construction business was started in central California in 1945 by Bill and Gerald's parents, William Jr. and Elizabeth Lyles. It has since grown into one of the larger California contractors, specializing in water and other infrastructure. Under their direction, Bill and Gerald Lyles have expanded the enterprise into real estate development, real estate rentals and agriculture. They credit their Purdue civil engineering education for much of their success. Bill Lyles received his bachelor's degree in civil engineering in 1955. Gerald Lyles received his bachelor's degree in civil engineering in 1964 and also earned a master's in industrial administration from Krannert School of Management in 1971. Bill's son, Will Lyles, earned bachelor's degrees in civil engineering and economics from Purdue in 1981. Other members of the Lyles family are Purdue alumni, including William Jr., who earned a bachelor's in civil engineering in 1935; Bill and Gerald's mother, Elizabeth, who earned a bachelor's degree in science in 1933 and a master's degree in education in 1934; and Bill and Gerald's sister Marybeth Lyles-Porter Seay, who earned a bachelor's degree in speech, language and hearing sciences in 1959. Their maternal grandfather, Henry Gerald Venemann, was a professor in the School of Mechanical Engineering. The family previously contributed to the Lyles Family Ideas to Innovation Learning Laboratory (i2i) in the School of Civil Engineering. In 1992 their mother funded the William M. Lyles Computational Laboratory, used by the School of Civil Engineering and the Division of Construction Engineering and Management. Lyles-Porter Hall, now under construction, is named in honor of a $10 million 2009 gift from Marybeth Lyles-Porter Seay. The new building will house the Department of Speech, Language and Hearing Sciences, the Indiana University School of Medicine-Lafayette and other health programs.";


	if(pipwrite(pip, words, 7500) == SYSERR) {
		kprintf("writer error\n");
	}
	if(pipdisconnect(pip) == SYSERR) {
		kprintf("writer disc error\n");
	}
	kprintf("writer disconnected!!!");
	sleep(5);
}

void owner_func(char* name, int sleeptime) {
	pid32 reader, writer;
	pipid32 pip;

	if((pip = pipcreate()) == SYSERR) {
		kprintf("owner create error\n");
	}

	reader = create ((void *)reader_func, INITSTK, 25, "B", 2, "B", pip);
	writer = create ((void *)writer_func, INITSTK, 30, "C", 2, "C", pip);

	
	if(pipconnect(pip, writer, reader) == SYSERR) {
		kprintf("owner connect error\n");
	}
	kprintf("pipe connected!\n");
	resume(reader);
	resume(writer);
	kprintf("owner");
	sleep(10);
	if(pipdelete(pip) == SYSERR) {
		kprintf("owner delete error\n");
	}
	kprintf("PIPE DELETED\n");
		
}

int main(int argc, char **argv)
{
	umsg32 retval;

	//resume(create ((void *)owner_func, INITSTK, 20, "A", 2, "A", 1));
		

	/* Creating a shell process */

	resume(create(shell, 4096, 1, "shell", 1, CONSOLE));

	retval = recvclr();
	while (TRUE) {
		retval = receive();
		kprintf("\n\n\rMain process recreating shell\n\n\r");
		resume(create(shell, 4096, 1, "shell", 1, CONSOLE));
	}

	return OK;
}
