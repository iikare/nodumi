wild = $(strip $(foreach d,$(wildcard $1*),$(call wild,$d/,$2) $(filter $(subst *,%,$2),$d)))
NAME = $(basename $(call wild,./,*.tex))

all:
	$(MAKE) $(NAME)
	$(MAKE) $(NAME)
	$(MAKE) clean

$(NAME):
	echo $(NAME)
	xelatex --shell-escape $(NAME).tex

clean:
	rm -f $(NAME).aux $(NAME).log texput.log $(NAME).out $(NAME).nav $(NAME).snm $(NAME).toc missfont.log
	rm -rf _minted* svg-inkscape
