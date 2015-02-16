git clone https://github.com/sstephenson/rbenv.git ~/.rbenv

git clone https://github.com/sstephenson/rbenv.git ~/.zshrc

echo 'eval "$(rbenv init -)"' >> ~/.zshrc

git clone https://github.com/sstephenson/ruby-build.git ~/.rbenv/plugins/ruby-build

rbenv install 2.1.5
rbenv global 2.1.5

gem install bundle




