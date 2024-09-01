#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "byos.h"

char *lsl[] = { "ls", "-l", "/proc/self/fd", NULL };
char *gosleep[] = { "sleep", "50000", NULL };
char *catf1[] = { "cat", "f1", NULL };
char *catbyos[] = { "cat", "byos.c", NULL };
char *cal[] = { "cal", NULL };


void nested_test_1() {
  // {
  //   ls -l
  //   { ls -l ; echo B > f1 ; cat f1 ; } > f2
  //   echo C
  // }
  int r;
  struct cmd innerarray[4] = {
    { .redir_stdout = NULL,
      .type = FORX,
      .data = { .forx = { "ls", lsl } }
    },
    { .redir_stdout = "f1",
      .type = ECHO,
      .data = { .echo = { "B\n" } }
    },
	{
		.redir_stdout = NULL,
		.type = ECHO,
		.data = { .echo = {"Meeeeeee\n"} }
	},
    { .redir_stdout = NULL,
      .type = FORX,
      .data = { .forx = { "cat", catf1 } }
    }
  };
  struct cmd outerarray[3] = {
    { .redir_stdout = NULL,
      .type = FORX,
      .data = { .forx = { "ls", lsl } }
    },
    { .redir_stdout = "f2",
      .type = LIST,
      .data = { .list = { 4, innerarray } }
    },
    { .redir_stdout = NULL,
      .type = ECHO,
      .data = { .echo = { "C\n" } }
    }
  };
  struct cmd example1 = {
    .redir_stdout = NULL,
    .type = LIST,
    .data = { .list = { 3, outerarray } }
  };

  r = interp(&example1);
  printf("return value = %d\n", r);
  // Expected outermost stdout:
  // A
  // C
  // Expected return value: 0
  // Expected f1:
  // B
  // Expected f2:
  // <output of ls -l, followed by:>
  // B   # because of "cat f1"
}

void nested_test_2() { 
  /*
   * {
   * 	{
   *		ls -l
   *		{ 
   *			ls -l
   *		}
   *		echo B > f1
   *		ls -l
   *		cat f1
   * 	} > f2
   * }
   * */ 
  struct cmd group2[] = {
	/*{
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"ls", lsl }}
	},*/
	{
		.redir_stdout = "f3",
		.type = FORX,
		.data = { .forx = {"cal", cal}}
	}
  };

  struct cmd group1[] = {
	{
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = { "ls", lsl }}
	},
	/*{
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"cat", catbyos }}
	},*/
	/*{
		.redir_stdout = NULL,
		.type = LIST,
		.data = { .list = {1, group2} }
	}*/
  };

  struct cmd innerarray[] = {
	{
		.redir_stdout = NULL,
		.type = FORX,
		.data = {.forx = {"ls", lsl} }
	},
	/*{
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"sleep", sleep }}
	},*/
	{ .redir_stdout = NULL,
	  .type = LIST,
	  .data = { .list = {1, group1} }
	},
	{ .redir_stdout = "f1",
	  .type = ECHO,
	  .data = { .echo = { "B\n" } }
	},
	{
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = { "ls", lsl }}
	},
	{ .redir_stdout = NULL,
	  .type = FORX,
	  .data = { .forx = { "cat", catf1 } }
	}
  };
  struct cmd outerarray[] = {
	{ .redir_stdout = "f2",
	  .type = LIST,
	  .data = { .list = { 5, innerarray } }
	},
  };
  struct cmd example1 = {
	.redir_stdout = NULL,
	.type = LIST,
	.data = { .list = { 1, outerarray } }
  };

  int r = interp(&example1);
  printf("return value = %d\n", r);
  // Expected outermost stdout:
  // Expected return value: 0
  // Expected f1:
  // B
  // Expected f2:
  // <output of ls -l, should be 4 fd opens, 3 for system, 1 because of ls>
  // B   # because of "cat f1"
}

void nested_test_3() {
  /*{
   * 	echo should be in stdout (1)
   * 		{
   *			ls -l
   *			{
   *				echo should be in f4 > f4
   *				echo but this one should be in f2 > f2
   *				{
   *					echo more nested in f4
   *					ls -l
   *				} > f4
   *			} > f3
   *			ls -l > f1
   *			ls -l > f3
   * 		} > f1
   * 	} > f1
   * 	echo should be in stdout (2) 
   * }
   * */
   
   struct cmd g4[] = {
	   {
		   .redir_stdout = NULL,
		   .type = ECHO,
		   .data = { .echo = { "more nested echo in f4\n"}}
	   },
	   {
		   .redir_stdout = NULL,
		   .type = FORX,
		   .data = { .forx = {"ls", lsl }}
	   }
   };

   struct cmd g3[] = {
	   {
		   .redir_stdout = "f4",
		   .type = ECHO,
		   .data = { .echo = {"echo should be in f4\n"}}
	   },
	   {
		   .redir_stdout = "f2",
		   .type = ECHO,
		   .data = { .echo = {"echo but this should be in f2\n"}}
	   },
	   {
		   .redir_stdout = "f4",
		   .type = LIST,
		   .data = { .list = {sizeof(g4)/sizeof(cmd), g4}}
	   },
   };

   struct cmd g2[] = {
	   {
		   .redir_stdout = NULL,
		   .type = FORX,
		   .data = { .forx = {"ls", lsl} }
	   },
	   {
		   .redir_stdout = "f3",
		   .type = LIST,
		   .data = { .list = { sizeof(g3) / sizeof(cmd), g3} }
	   },
	   {
		   .redir_stdout = "f1",
		   .type = FORX,
		   .data = { .forx = {"ls", lsl} }
	   },
	   {
		   .redir_stdout = "f3",
		   .type = FORX,
		   .data = {.forx = {"ls", lsl}}
	   }
   };

   struct cmd g1[] = {
	   {
		   .redir_stdout = NULL,
		   .type = ECHO,
		   .data = { "echo should be in stdout (1)\n" }
	   },
	   {
		   .redir_stdout = "f1",
		   .type = LIST,
		   .data = { .list = { sizeof(g2)/sizeof(cmd), g2 } }
	   },
	   {
		   .redir_stdout = NULL,
		   .type = ECHO,
		   .data = { "echo should be in stdout (2)\n" }
	   }

   };

   struct cmd example1[] = {
	   {
		   .redir_stdout = NULL,
		   .type = LIST,
		   .data = { .list = {sizeof(g1)/sizeof(cmd), g1 }}
	   }
   };

  int r = interp(example1);
  printf("return value = %d\n", r);
}

void put_nested_cmd(int n, struct cmd *c) {
	if (n == 1025) {
		return;
	}
	char *fn = calloc(5, sizeof(char)), fi[5];
	strcpy(fn, "f");
	sprintf(fi, "%d", n);
	strcat(fn, fi);
	struct cmd *newcmd = calloc(2, sizeof(struct cmd));

	newcmd[0] = (struct cmd){
		.redir_stdout = NULL,
		.type = FORX,
//		.data = { "Echo to file!\n" }i
		.data = { .forx = { "ls", lsl } }
	};
	newcmd[1] = (struct cmd){
		.redir_stdout = fn,
		.type = LIST,
		.data = { .list = {} }
	};
	if (n > 2) {
		c[1].data.list.cmds = newcmd;
		c[1].data.list.n = 2;
	}
	else {
		c[0].data.list.cmds = newcmd;
		c[0].data.list.n = 2;
	}
	put_nested_cmd(n + 1, newcmd);
}

void cleanup(int n, struct cmd *command) {
	if (n == 5) {
		free(command[1].redir_stdout);
		free(command);
		return;
	}	
	cleanup(n + 1, command[1].data.list.cmds);
	if (n != 1) {
		free(command[1].redir_stdout);
		free(command);
	}
}


void nested_test_4() {
	struct cmd command = {	
		.redir_stdout = "f1",
		.type = LIST,
		.data = { .list = {} }
	};
	put_nested_cmd(2, &command);
	int r = interp(&command);
	printf("return value = %d\n", r);
	cleanup(2, command.data.list.cmds);
}

void test_echo_1() {
	struct cmd test = {
		.redir_stdout = NULL,
		.type = ECHO,
		.data = "Hello",
	};
	
	int r = interp(&test);
	printf("return value = %d\n", r);
	/* Expected stdout:
	   Hello
	   return value = 0
	 * */
}

void test_echo_2() {
	struct cmd test = {
		.redir_stdout = "echo_2",
		.type = ECHO,
		.data = "Hello"
	};

	int r = interp(&test);
	printf("return value = %d\n", r);
	/* Expected 'echo_2':
	   Hello
	 * */
}

void test_kill1() {
	char *kill2[] = { "silly", "-2", NULL };
	struct cmd test_kill = {
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"./silly", kill2 }}
	};
	int r = interp(&test_kill);
	assert(r == 130);
}

void test_kill2() {
	char *kill2[] = { "silly", "-15", NULL };
	struct cmd test_kill = {
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"./silly", kill2 }}
	};
	int r = interp(&test_kill);
	assert(r == 143);
}

void test_kill3() {
	char *kill2[] = { "silly", "-9", NULL };
	struct cmd test_kill = {
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"./silly", kill2 }}
	};
	int r = interp(&test_kill);
	assert(r == 137);
}

void test_kill4() {
	char *kill2[] = { "silly", "-3", NULL };
	struct cmd test_kill = {
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"./silly", kill2 }}
	};
	int r = interp(&test_kill);
	assert(r == 131);
}

void test_exit_code1() {
	char *kill2[] = { "silly", "10", NULL };
	struct cmd test_kill = {
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"./silly", kill2 }}
	};
	int r = interp(&test_kill);
	assert(r == 10);
}

void test_exit_code2() {
	char *kill2[] = { "silly", "1000", NULL };
	struct cmd test_kill = {
		.redir_stdout = NULL,
		.type = FORX,
		.data = { .forx = {"./silly", kill2 }}
	};
	int r = interp(&test_kill);
	assert(r == 232);
}

int main(void){
  //nested_test_1();
  //nested_test_2();
  /*nested_test_3();
  test_kill1();
  test_kill2();
  test_kill3();
  test_kill4();
  test_exit_code1();
  test_exit_code2();*/
 	nested_test_4();
  /*test_echo_1();
  test_echo_2();*/
  /*int fd = STDOUT_FILENO;
  char a[10];
  read(fd, a, 10);
  printf("This is from c program: %s", a);*/
  return 0;
}

